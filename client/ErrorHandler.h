#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H

#include <string>

/**
 * @brief Класс для обработки ошибок и вывода сообщений
 * @details Содержит статические методы для логирования ошибок, 
 * аварийного завершения программы и вывода справки.
 * @warning Все методы являются статическими, экземпляры класса не создаются.
 * @author Ежов Егор Александрович
 * @date 01.12.2025
 * @version 1.0
 */
class ErrorHandler {
public:
    /**
     * @brief Логирует сообщение об ошибке
     * @param [in] message Текст сообщения об ошибке
     */
    static void logError(const std::string& message);

    /**
     * @brief Выводит сообщение об ошибке и завершает программу
     * @param [in] message Текст сообщения об ошибке
     */
    static void exitWithError(const std::string& message);

    /**
     * @brief Выводит справочную информацию
     */
    static void printHelp();
};

#endif // ERRORHANDLER_H
