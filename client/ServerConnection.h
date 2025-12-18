#ifndef SERVERCONNECTION_H
#define SERVERCONNECTION_H

#include <string>
#include <vector>

/**
 * @brief Класс для управления подключением к серверу
 * @details Обеспечивает установку соединения, аутентификацию,
 * отправку и получение данных (текстовых и бинарных), а также отправку
 * векторов для обработки и получение результатов.
 * @author Ежов Егор Александрович
 * @date 01.12.2025
 * @version 1.0
 */
class ServerConnection {
private:
    int socketFD;              ///< Дескриптор сокета
    std::string login;         ///< Логин пользователя
    std::string password;      ///< Пароль пользователя
    
    /**
     * @brief Отправляет текстовые данные через сокет
     * @param [in] text Текст для отправки
     * @return true если отправка успешна, false в случае ошибки
     */
    bool sendText(const std::string& text);
    
    /**
     * @brief Принимает текстовые данные через сокет
     * @param [out] text Буфер для принятого текста
     * @return true если прием успешен, false в случае ошибки
     */
    bool receiveText(std::string& text);
    
    /**
     * @brief Отправляет бинарные данные через сокет
     * @param [in] data Указатель на данные
     * @param [in] size Размер данных в байтах
     * @return true если отправка успешна, false в случае ошибки
     */
    bool sendBinaryData(const void* data, size_t size);
    
    /**
     * @brief Принимает бинарные данные через сокет
     * @param [out] data Буфер для принятых данных
     * @param [in] size Ожидаемый размер данных в байтах
     * @return true если прием успешен, false в случае ошибки
     */
    bool receiveBinaryData(void* data, size_t size);
    
public:
    /**
     * @brief Конструктор класса ServerConnection
     */
    ServerConnection();
    
    /**
     * @brief Деструктор класса ServerConnection
     * @details Автоматически закрывает соединение при уничтожении объекта
     */
    ~ServerConnection();
    
    /**
     * @brief Устанавливает соединение с сервером
     * @param [in] address Адрес сервера (IP или доменное имя)
     * @param [in] port Порт сервера
     * @return true если соединение установлено, false в случае ошибки
     */
    bool establishConnection(const std::string& address, int port);
    
    /**
     * @brief Выполняет аутентификацию на сервере
     * @param [in] login Логин пользователя
     * @param [in] password Пароль пользователя
     * @return true если аутентификация успешна, false в случае ошибки
     */
    bool authenticate(const std::string& login, const std::string& password);
    
    /**
     * @brief Отправляет векторы на сервер для обработки и получает результаты
     * @param [in] vectors Векторы для обработки
     * @param [out] results Результаты обработки от сервера
     * @return true если операция успешна, false в случае ошибки
     */
    bool sendVectors(const std::vector<std::vector<double>>& vectors, std::vector<double>& results);
    
    /**
     * @brief Закрывает соединение с сервером
     */
    void closeConnection();
};

#endif // SERVERCONNECTION_H
