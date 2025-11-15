#include "client.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <algorithm>
#include <stdexcept>
#include <cstdlib>

std::string NetworkClient::read_auth_credentials() {
    std::string actual_auth_file = auth_file;
    if (auth_file.find("~/") == 0) {
        const char* home = getenv("HOME");
        if (home) {
            actual_auth_file = std::string(home) + auth_file.substr(1);
        }
    }
    
    std::ifstream file(actual_auth_file);
    if (!file.is_open()) {
        throw std::runtime_error("Не удалось открыть файл аутентификации: " + actual_auth_file);
    }
    
    std::string login, password;
    std::getline(file, login);
    std::getline(file, password);
    
    file.close();
    
    login.erase(0, login.find_first_not_of(" \t\n\r"));
    login.erase(login.find_last_not_of(" \t\n\r") + 1);
    password.erase(0, password.find_first_not_of(" \t\n\r"));
    password.erase(password.find_last_not_of(" \t\n\r") + 1);
    
    if (login.empty() || password.empty()) {
        throw std::runtime_error("Неверный формат файла аутентификации");
    }
    
    return login + ":" + password;
}

bool NetworkClient::authenticate() {
    std::string credentials = read_auth_credentials();
    size_t colon_pos = credentials.find(':');
    if (colon_pos == std::string::npos) {
        throw std::runtime_error("Неверный формат учетных данных");
    }
    
    std::string login = credentials.substr(0, colon_pos);
    std::string password = credentials.substr(colon_pos + 1);
    
    std::cout << "Аутентификация пользователя: " << login << std::endl;
    
    if (send_data(login.c_str(), login.length()) <= 0) {
        throw std::runtime_error("Ошибка отправки LOGIN");
    }
    
    char buffer[256];
    int bytes_received = receive_data(buffer, sizeof(buffer) - 1);
    if (bytes_received <= 0) {
        throw std::runtime_error("Ошибка получения SALT");
    }
    
    buffer[bytes_received] = '\0';
    std::string response(buffer);
    
    if (response == "ERR") {
        throw std::runtime_error("Сервер отклонил LOGIN");
    }
    
    if (response.length() != 16) {
        throw std::runtime_error("Неверный формат SALT");
    }
    
    std::string salt = response;
    std::cout << "Получен SALT: " << salt << std::endl;
    
    std::string hash_input = salt + password;
    std::string hash_result = sha1_hash(hash_input);
    
    for (char& c : hash_result) {
        c = std::toupper(c);
    }
    
    std::cout << "Вычисленный HASH: " << hash_result << std::endl;
    
    if (send_data(hash_result.c_str(), hash_result.length()) <= 0) {
        throw std::runtime_error("Ошибка отправки HASH");
    }
    
    bytes_received = receive_data(buffer, sizeof(buffer) - 1);
    if (bytes_received <= 0) {
        throw std::runtime_error("Ошибка получения результата аутентификации");
    }
    
    buffer[bytes_received] = '\0';
    response = buffer;
    
    if (response == "OK") {
        std::cout << "Аутентификация успешна" << std::endl;
        return true;
    } else {
        throw std::runtime_error("Ошибка аутентификации: " + response);
    }
}
