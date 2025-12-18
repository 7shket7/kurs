/**
 * @file ErrorHandler.cpp
 * @brief Реализация класса ErrorHandler
 * @details Содержит реализацию методов для обработки ошибок, логирования и вывода справки.
 * @author Ежов Егор Александрович
 * @date 01.12.2025
 * @version 1.0
 */

#include "ErrorHandler.h"
#include <iostream>
#include <cstdlib>

/**
 * @brief Логирует сообщение об ошибке
 * @param [in] message Текст сообщения об ошибке
 * @details Выводит сообщение об ошибке в стандартный поток ошибок (stderr)
 * с префиксом "[ОШИБКА]".
 */
void ErrorHandler::logError(const std::string& message) {
    std::cerr << "[ОШИБКА] " << message << std::endl;
}

/**
 * @brief Выводит сообщение об ошибке и завершает программу
 * @param [in] message Текст сообщения об ошибке
 * @details Вызывает logError() для вывода сообщения, затем завершает программу
 * с кодом возврата EXIT_FAILURE.
 * @warning Приводит к немедленному завершению программы!
 */
void ErrorHandler::exitWithError(const std::string& message) {
    logError(message);
    std::exit(EXIT_FAILURE);
}

/**
 * @brief Выводит справочную информацию о программе
 * @details Выводит в стандартный поток вывода (stdout) информацию о синтаксисе
 * командной строки и доступных опциях клиентского приложения.
 */
void ErrorHandler::printHelp() {
    std::cout << "Использование: ./client <адрес_сервера> <входной_файл> <выходной_файл> [опции]\n";
    std::cout << "Опции:\n";
    std::cout << "  -p <порт>          Порт сервера (по умолчанию: 33333)\n";
    std::cout << "  -c <файл_конфига>  Файл с логином и паролем (по умолчанию: ~/.config/velient.conf)\n";
    std::cout << "  -h                 Показать эту справку\n";
}
