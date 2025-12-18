#ifndef DATAPROCESSOR_H
#define DATAPROCESSOR_H

#include <string>
#include <vector>

/**
 * @brief Класс для обработки данных (векторов)
 * @details Предоставляет функциональность для чтения векторов из файла,
 * валидации данных, преобразования в бинарный формат и сохранения результатов.
 * @author Ежов Егор Александрович
 * @date 01.12.2025
 * @version 1.0
 */
class DataProcessor {
private:
    std::vector<std::vector<double>> vectors;  ///< Коллекция векторов для обработки
    
public:
    /**
     * @brief Читает векторы из файла
     * @param [in] filename Имя файла с данными
     * @return true если чтение успешно, false в случае ошибки
     * @details Ожидает формат файла, где каждый вектор представлен
     * на отдельной строке, а числа разделены пробелами.
     */
    bool readVectorsFromFile(const std::string& filename);
    
    /**
     * @brief Проверяет корректность загруженных данных
     * @return true если данные корректны, false в случае ошибки
     * @details Выполняет проверки:
     * - Наличие хотя бы одного вектора
     * - Корректный размер всех векторов
     * - Корректные числовые значения
     */
    bool validateData() const;
    
    /**
     * @brief Преобразует векторы в бинарный формат
     * @return Бинарное представление векторов в виде вектора байтов
     * @details Формат бинарных данных соответствует протоколу сервера
     */
    std::vector<char> convertToBinary() const;
    
    /**
     * @brief Сохраняет результаты обработки в файл
     * @param [in] filename Имя файла для сохранения
     * @param [in] results Результаты обработки от сервера
     * @return true если сохранение успешно, false в случае ошибки
     */
    bool saveResults(const std::string& filename, const std::vector<double>& results) const;
    
    /**
     * @brief Возвращает константную ссылку на векторы
     * @return Константная ссылка на коллекцию векторов
     */
    const std::vector<std::vector<double>>& getVectors() const { return vectors; }
    
    /**
     * @brief Возвращает количество векторов
     * @return Количество загруженных векторов
     */
    size_t getVectorsCount() const { return vectors.size(); }
};

#endif // DATAPROCESSOR_H
