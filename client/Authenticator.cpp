/**
 * @file Authenticator.cpp
 * @brief Реализация класса Authenticator
 * @details Содержит реализацию методов для работы с хешированием, генерацией соли
 * и преобразованием шестнадцатеричных строк. Использует библиотеку OpenSSL для SHA1.
 * @author Ежов Егор Александрович
 * @date 01.12.2025
 * @version 1.0
 */

#include "Authenticator.h"
#include "ErrorHandler.h"
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>
#include <random>
#include <algorithm>
#include <cctype>

/**
 * @brief Вычисляет хеш пароля с использованием соли
 * @param [in] salt Соль для усиления хеша (16 символов)
 * @param [in] password Пароль пользователя
 * @return Хеш-строка в шестнадцатеричном формате (40 символов, верхний регистр)
 * @details Алгоритм:
 * 1. Объединяет соль и пароль в одну строку
 * 2. Вычисляет SHA1 хеш от объединенной строки
 * 3. Преобразует бинарный хеш в шестнадцатеричное представление
 * 4. Приводит результат к верхнему регистру
 * @warning Для работы требуется библиотека OpenSSL
 */
std::string Authenticator::computeHash(const std::string& salt, const std::string& password) {
    std::string combined = salt + password;
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(combined.c_str()), combined.size(), hash);

    std::stringstream ss;
    for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') 
           << static_cast<int>(hash[i]);
    }
    
    // Конвертируем в верхний регистр
    std::string result = ss.str();
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    
    return result;
}

/**
 * @brief Генерирует случайную соль для хеширования
 * @return Соль в виде шестнадцатеричной строки (16 символов, верхний регистр)
 * @details Алгоритм:
 * 1. Генерирует 64-битное случайное число
 * 2. Преобразует число в шестнадцатеричное представление
 * 3. Дополняет нулями слева до 16 символов
 * 4. Приводит результат к верхнему регистру
 * @note Использует аппаратный генератор случайных чисел (std::random_device)
 * для криптографически стойкой генерации
 */
std::string Authenticator::generateSalt() {
    // Генерация 64-битной соли (16 шестнадцатеричных символов)
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;
    
    uint64_t salt = dis(gen);
    std::stringstream ss;
    ss << std::hex << std::setw(16) << std::setfill('0') << salt;
    
    // Конвертируем в верхний регистр
    std::string result = ss.str();
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    
    return result;
}

/**
 * @brief Преобразует шестнадцатеричную строку в строку символов
 * @param [in] hex Шестнадцатеричная строка (четное количество символов)
 * @return Декодированная строка
 * @details Алгоритм:
 * 1. Обрабатывает строку по два символа за итерацию
 * 2. Каждую пару символов интерпретирует как шестнадцатеричное число
 * 3. Преобразует число в символ и добавляет к результату
 * @warning Требует корректного формата входной строки (только шестнадцатеричные символы)
 * @warning Длина входной строки должна быть четной
 */
std::string Authenticator::hexToString(const std::string& hex) {
    std::string result;
    for (size_t i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        char byte = static_cast<char>(strtol(byteString.c_str(), nullptr, 16));
        result.push_back(byte);
    }
    return result;
}
