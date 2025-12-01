#include "Client.h"
#include "ErrorHandler.h"
#include "Authenticator.h"
#include "DataProcessor.h"
#include "ServerConnection.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <pwd.h>

ClientConfig::ClientConfig() : serverPort(33333), configFileName("~/.config/velient.conf") {}

bool Client::parseCommandLineArgs(int argc, char* argv[]) {
    if (argc < 4) {
        ErrorHandler::printHelp();
        return false;
    }
    
    // Обязательные параметры
    config.serverAddress = argv[1];
    config.inputFileName = argv[2];
    config.outputFileName = argv[3];
    
    // Опциональные параметры
    for (int i = 4; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            config.serverPort = std::stoi(argv[++i]);
        } else if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
            config.configFileName = argv[++i];
        } else if (strcmp(argv[i], "-h") == 0) {
            ErrorHandler::printHelp();
            return false;
        } else {
            ErrorHandler::logError("Неизвестный параметр: " + std::string(argv[i]));
            ErrorHandler::printHelp();
            return false;
        }
    }
    
    return true;
}

bool Client::readConfigFile() {
    // Разворачиваем ~ в домашнюю директорию
    if (config.configFileName.find("~/") == 0) {
        const char* homeDir = getenv("HOME");
        if (!homeDir) {
            struct passwd* pw = getpwuid(getuid());
            homeDir = pw->pw_dir;
        }
        config.configFileName = std::string(homeDir) + config.configFileName.substr(1);
    }
    
    std::ifstream configFile(config.configFileName);
    if (!configFile.is_open()) {
        ErrorHandler::logError("Не удалось открыть файл конфигурации: " + config.configFileName);
        return false;
    }
    
    // Чтение логина и пароля из файла
    std::string login, password;
    
    // Читаем логин (первая непустая строка)
    std::string line;
    while (std::getline(configFile, line)) {
        // Удаляем пробелы в начале и конце
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue; // Пустая строка
        size_t end = line.find_last_not_of(" \t\r\n");
        line = line.substr(start, end - start + 1);
        
        if (line.empty() || line[0] == '#') continue; // Пропускаем комментарии
        
        login = line;
        break;
    }
    
    if (login.empty()) {
        ErrorHandler::logError("Логин не найден в файле конфигурации: " + config.configFileName);
        return false;
    }
    
    // Читаем пароль (следующая непустая строка)
    while (std::getline(configFile, line)) {
        // Удаляем пробелы в начале и конце
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue; // Пустая строка
        size_t end = line.find_last_not_of(" \t\r\n");
        line = line.substr(start, end - start + 1);
        
        if (line.empty() || line[0] == '#') continue; // Пропускаем комментарии
        
        password = line;
        break;
    }
    
    if (password.empty()) {
        ErrorHandler::logError("Пароль не найден в файле конфигурации: " + config.configFileName);
        return false;
    }
    
    config.login = login;
    config.password = password;
    
    configFile.close();
    std::cout << "Лог: Прочитан логин: " << config.login << std::endl;
    return true;
}

bool Client::run(int argc, char* argv[]) {
    // 1. Парсинг аргументов командной строки
    if (!parseCommandLineArgs(argc, argv)) {
        return false;
    }
    
    // 2. Чтение конфигурационного файла
    if (!readConfigFile()) {
        return false;
    }
    
    // 3. Обработка данных
    DataProcessor dataProcessor;
    if (!dataProcessor.readVectorsFromFile(config.inputFileName)) {
        ErrorHandler::exitWithError("Ошибка чтения векторов из файла");
        return false;
    }
    
    if (!dataProcessor.validateData()) {
        ErrorHandler::exitWithError("Ошибка валидации данных");
        return false;
    }
    
    std::cout << "Отладка: Прочитано " << dataProcessor.getVectorsCount() << " векторов из файла " << config.inputFileName << std::endl;
    
    // 4. Установка соединения с сервером
    ServerConnection connection;
    if (!connection.establishConnection(config.serverAddress, config.serverPort)) {
        ErrorHandler::exitWithError("Ошибка установки соединения с сервером");
        return false;
    }
    
    // 5. Аутентификация
    if (!connection.authenticate(config.login, config.password)) {
        ErrorHandler::exitWithError("Ошибка аутентификации");
        connection.closeConnection();
        return false;
    }
    
    // 6. Получение векторов и их отправка
    const std::vector<std::vector<double>>& vectors = dataProcessor.getVectors();
    std::vector<double> results;
    
    if (!connection.sendVectors(vectors, results)) {
        ErrorHandler::exitWithError("Ошибка отправки векторов на сервер");
        connection.closeConnection();
        return false;
    }
    
    std::cout << "Лог: Получено " << results.size() << " результатов от сервера" << std::endl;
    
    // Проверяем количество результатов
    if (results.size() != vectors.size()) {
        std::cout << "Предупреждение: получено " << results.size() << " результатов, ожидалось " << vectors.size() << std::endl;
    }
    
    // 7. Сохранение результатов
    if (!dataProcessor.saveResults(config.outputFileName, results)) {
        ErrorHandler::exitWithError("Ошибка сохранения результатов");
        connection.closeConnection();
        return false;
    }
    
    // 8. Закрытие соединения
    connection.closeConnection();
    
    std::cout << "Программа завершена успешно. Результаты сохранены в " << config.outputFileName << std::endl;
    return true;
}
