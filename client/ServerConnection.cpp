#include "ServerConnection.h"
#include "ErrorHandler.h"
#include "Authenticator.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <cstring>
#include <iostream>
#include <cstdint>
#include <errno.h>

// Вспомогательные функции для преобразования порядка байтов для 64-битных значений
#if __BYTE_ORDER == __LITTLE_ENDIAN

uint64_t htonll(uint64_t value) {
    return ((uint64_t)htonl(value & 0xFFFFFFFF) << 32) | htonl(value >> 32);
}

uint64_t ntohll(uint64_t value) {
    return ((uint64_t)ntohl(value & 0xFFFFFFFF) << 32) | ntohl(value >> 32);
}
#else
uint64_t htonll(uint64_t value) { return value; }
uint64_t ntohll(uint64_t value) { return value; }
#endif

ServerConnection::ServerConnection() : socketFD(-1) {}

ServerConnection::~ServerConnection() {
    closeConnection();
}

bool ServerConnection::establishConnection(const std::string& address, int port) {
    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD < 0) {
        ErrorHandler::logError("Ошибка создания сокета");
        return false;
    }
    
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, address.c_str(), &serverAddr.sin_addr) <= 0) {
        ErrorHandler::logError("Неверный адрес сервера: " + address);
        return false;
    }
    
    if (connect(socketFD, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        ErrorHandler::logError("Не удалось подключиться к серверу " + address + ":" + std::to_string(port));
        return false;
    }
    
    std::cout << "Лог: Установлено соединение с " << address << ":" << port << std::endl;
    return true;
}

bool ServerConnection::sendText(const std::string& text) {
    std::string message = text + "\n";
    ssize_t bytesSent = send(socketFD, message.c_str(), message.length(), 0);
    
    if (bytesSent != static_cast<ssize_t>(message.length())) {
        ErrorHandler::logError("Ошибка отправки текста: " + text);
        return false;
    }
    
    return true;
}

bool ServerConnection::receiveText(std::string& text) {
    char buffer[1024];
    ssize_t bytesReceived = recv(socketFD, buffer, sizeof(buffer) - 1, 0);
    
    if (bytesReceived <= 0) {
        ErrorHandler::logError("Ошибка получения текста от сервера");
        return false;
    }
    
    buffer[bytesReceived] = '\0';
    text = buffer;
    
    size_t newlinePos = text.find('\n');
    if (newlinePos != std::string::npos) {
        text = text.substr(0, newlinePos);
    }
    
    return true;
}

bool ServerConnection::authenticate(const std::string& userLogin, const std::string& userPassword) {
    login = userLogin;
    password = userPassword;
    
    // Отправка LOGIN
    std::cout << "Лог: Отправка LOGIN: " << login << std::endl;
    if (!sendText(login)) {
        ErrorHandler::logError("Ошибка отправки LOGIN");
        return false;
    }
    
    // Получение SALT16
    std::string saltResponse;
    if (!receiveText(saltResponse)) {
        ErrorHandler::logError("Ошибка получения SALT от сервера");
        return false;
    }
    
    if (saltResponse == "ERR") {
        ErrorHandler::logError("Сервер отверг идентификацию");
        return false;
    }
    
    if (saltResponse.length() != 16) {
        ErrorHandler::logError("Неверный формат SALT: " + saltResponse);
        return false;
    }
    
    // Вычисление и отправка HASH
    std::string hash = Authenticator::computeHash(saltResponse, password);
    std::cout << "Лог: Отправка HASH: " << hash << std::endl;
    
    if (!sendText(hash)) {
        ErrorHandler::logError("Ошибка отправки HASH");
        return false;
    }
    
    // Получение ответа об аутентификации
    std::string authResponse;
    if (!receiveText(authResponse)) {
        ErrorHandler::logError("Ошибка получения ответа аутентификации");
        return false;
    }
    
    if (authResponse != "OK") {
        ErrorHandler::logError("Ошибка аутентификации: " + authResponse);
        return false;
    }
    
    std::cout << "Лог: Аутентификация успешна" << std::endl;
    return true;
}

bool ServerConnection::sendBinaryData(const void* data, size_t size) {
    ssize_t totalSent = 0;
    const char* dataPtr = static_cast<const char*>(data);
    
    while (totalSent < static_cast<ssize_t>(size)) {
        ssize_t sent = send(socketFD, dataPtr + totalSent, size - totalSent, 0);
        if (sent <= 0) {
            ErrorHandler::logError("Ошибка отправки бинарных данных");
            return false;
        }
        totalSent += sent;
    }
    
    return true;
}

bool ServerConnection::receiveBinaryData(void* data, size_t size) {
    ssize_t totalReceived = 0;
    char* dataPtr = static_cast<char*>(data);
    
    while (totalReceived < static_cast<ssize_t>(size)) {
        ssize_t received = recv(socketFD, dataPtr + totalReceived, size - totalReceived, 0);
        if (received <= 0) {
            ErrorHandler::logError("Ошибка получения бинарных данных");
            return false;
        }
        totalReceived += received;
    }
    
    return true;
}

bool ServerConnection::sendVectors(const std::vector<std::vector<double>>& vectors, std::vector<double>& results) {
    results.clear();
    results.reserve(vectors.size());
    uint32_t numVectors = static_cast<uint32_t>(vectors.size());
    
    // Отладка: показываем что отправляем
    std::cout << "Отладка: Отправляем " << numVectors << " векторов" << std::endl;
    std::cout << "Отладка: Байты количества векторов (hex): ";
    unsigned char* bytes = reinterpret_cast<unsigned char*>(&numVectors);
    for (size_t i = 0; i < sizeof(numVectors); i++) {
        printf("%02X ", bytes[i]);
    }
    std::cout << std::endl;
    
    if (!sendBinaryData(&numVectors, sizeof(numVectors))) {
        ErrorHandler::logError("Ошибка отправки количества векторов");
        return false;
    }
    
    // 2. Для каждого вектора
    for (size_t i = 0; i < vectors.size(); ++i) {
        const auto& vec = vectors[i];
        uint32_t vecSize = static_cast<uint32_t>(vec.size());
        
        std::cout << "Отладка: Размер вектора " << i << ": " << vecSize << std::endl;
        std::cout << "Отладка: Байты размера вектора (hex): ";
        bytes = reinterpret_cast<unsigned char*>(&vecSize);
        for (size_t j = 0; j < sizeof(vecSize); j++) {
            printf("%02X ", bytes[j]);
        }
        std::cout << std::endl;
        
        if (!sendBinaryData(&vecSize, sizeof(vecSize))) {
            ErrorHandler::logError("Ошибка отправки размера вектора " + std::to_string(i));
            return false;
        }
        
        if (vecSize > 0) {
            // Выводим первые значения для отладки
            if (i == 0 && vecSize > 0) {
                std::cout << "Отладка: Первые 2 значения вектора 0: ";
                for (size_t j = 0; j < std::min(vecSize, (uint32_t)2); ++j) {
                    std::cout << vec[j] << " ";
                }
                std::cout << std::endl;
            }
            
            // Отправляем значения как есть (little-endian)
            if (!sendBinaryData(vec.data(), vecSize * sizeof(double))) {
                ErrorHandler::logError("Ошибка отправки значений вектора " + std::to_string(i));
                return false;
            }
        }
        
        // 3. Получаем результат для этого вектора
        // Ждем некоторое время, чтобы сервер успел обработать
        usleep(1000);
        
        double result;
        ssize_t bytesReceived = recv(socketFD, &result, sizeof(result), MSG_DONTWAIT);
        
        if (bytesReceived == sizeof(result)) {
            // Успешно получили результат
            std::cout << "Отладка: Получен результат для вектора " << i << ": " << result << std::endl;
            results.push_back(result);
        } else if (bytesReceived == 0) {
            // Сервер закрыл соединение
            ErrorHandler::logError("Сервер закрыл соединение после вектора " + std::to_string(i));
            return false;
        } else if (bytesReceived == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            // Нет данных, пробуем снова с ожиданием
            bytesReceived = recv(socketFD, &result, sizeof(result), MSG_WAITALL);
            if (bytesReceived == sizeof(result)) {
                std::cout << "Отладка: Получен результат для вектора " << i << " (с ожиданием): " << result << std::endl;
                results.push_back(result);
            } else {
                ErrorHandler::logError("Ошибка получения результата для вектора " + std::to_string(i));
                return false;
            }
        } else {
            ErrorHandler::logError("Ошибка получения результата для вектора " + std::to_string(i));
            return false;
        }
    }
    
    std::cout << "Лог: Успешно отправлено " << vectors.size() << " векторов и получено " << results.size() << " результатов" << std::endl;
    return true;
}

void ServerConnection::closeConnection() {
    if (socketFD >= 0) {
        std::cout << "Лог: Закрытие соединения" << std::endl;
        close(socketFD);
        socketFD = -1;
    }
}
