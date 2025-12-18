/**
 * @file main.cpp
 * @brief Главный файл клиентского приложения
 * @details Содержит точку входа в программу и основную логику запуска клиента.
 * Поддерживает работу с аргументами командной строки.
 * @author Ежов Егор Александрович
 * @date 01.12.2025
 * @version 1.0
 */

#include "Client.h"
#include "ErrorHandler.h"

/**
 * @brief Точка входа в программу
 * @param [in] argc Количество аргументов командной строки
 * @param [in] argv Массив аргументов командной строки
 * @return Код завершения программы:
 *         - EXIT_SUCCESS (0) - успешное выполнение
 *         - EXIT_FAILURE (1) - ошибка выполнения
 * @details Логика работы:
 * 1. Если аргументы не указаны (argc == 1) - выводится справочная информация.
 * 2. Создается экземпляр класса Client.
 * 3. Выполняется запуск клиента с передачей аргументов.
 * 4. В случае ошибки возвращается EXIT_FAILURE.
 */
int main(int argc, char* argv[]) {
    // Если аргументы не указаны, выводим справку
    if (argc == 1) {
        ErrorHandler::printHelp();
        return EXIT_SUCCESS;
    }
    
    // Создаем и запускаем клиент
    Client client;
    if (!client.run(argc, argv)) {
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
