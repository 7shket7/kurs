#ifndef AUTHENTICATOR_H
#define AUTHENTICATOR_H

#include <string>
#include <vector>

/**
 * @brief Класс для аутентификации и работы с хешированием паролей
 * @details Предоставляет статические методы для вычисления хеша пароля с солью,
 * генерации соли и преобразования шестнадцатеричных строк.
 * @warning Все методы являются статическими, экземпляры класса не создаются.
 * @author Ежов Егор Александрович
 * @date 01.12.2025
 * @version 1.0
 */
class Authenticator {
public:
    /**
     * @brief Вычисляет хеш пароля с использованием соли
     * @param [in] salt Соль для усиления хеша
     * @param [in] password Пароль пользователя
     * @return Хеш-строка в шестнадцатеричном формате
     */
    static std::string computeHash(const std::string& salt, const std::string& password);

    /**
     * @brief Генерирует случайную соль для хеширования
     * @return Соль в виде шестнадцатеричной строки
     */
    static std::string generateSalt();

    /**
     * @brief Преобразует шестнадцатеричную строку в строку символов
     * @param [in] hex Шестнадцатеричная строка
     * @return Декодированная строка
     * @warning Требует корректного формата входной строки
     */
    static std::string hexToString(const std::string& hex);
};

#endif // AUTHENTICATOR_H
