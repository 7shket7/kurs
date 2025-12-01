#include "DataProcessor.h"
#include "ErrorHandler.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>
#include <cstdint>

bool DataProcessor::readVectorsFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        ErrorHandler::exitWithError("Не удалось открыть файл: " + filename);
        return false;
    }

    size_t numVectors;
    file >> numVectors;
    
    if (!file || numVectors == 0) {
        ErrorHandler::exitWithError("Ошибка чтения количества векторов из файла: " + filename);
        return false;
    }

    vectors.clear();
    vectors.reserve(numVectors);

    for (size_t i = 0; i < numVectors; ++i) {
        size_t vectorSize;
        file >> vectorSize;
        
        if (!file || vectorSize == 0) {
            ErrorHandler::exitWithError("Ошибка чтения размера вектора " + std::to_string(i+1));
            return false;
        }

        std::vector<double> vec;
        vec.reserve(vectorSize);
        
        for (size_t j = 0; j < vectorSize; ++j) {
            double value;
            file >> value;
            if (!file) {
                ErrorHandler::exitWithError("Ошибка чтения значения вектора " + std::to_string(i+1));
                return false;
            }
            vec.push_back(value);
        }
        
        vectors.push_back(vec);
        
        std::cout << "Отладка: Вектор " << i << ", размер " << vectorSize << std::endl;
    }

    file.close();
    return true;
}

bool DataProcessor::validateData() const {
    if (vectors.empty()) {
        ErrorHandler::logError("Нет векторов для обработки");
        return false;
    }
    
    for (const auto& vec : vectors) {
        if (vec.empty()) {
            ErrorHandler::logError("Обнаружен пустой вектор");
            return false;
        }
    }
    
    return true;
}

std::vector<char> DataProcessor::convertToBinary() const {
    std::vector<char> binaryData;
    
    std::cout << "Отладка: Отправляем " << vectors.size() << " векторов" << std::endl;
    
    // Количество векторов
    uint32_t numVectors = static_cast<uint32_t>(vectors.size());
    char* numVectorsPtr = reinterpret_cast<char*>(&numVectors);
    binaryData.insert(binaryData.end(), numVectorsPtr, numVectorsPtr + sizeof(uint32_t));
    
    // Каждый вектор
    for (size_t i = 0; i < vectors.size(); ++i) {
        const auto& vec = vectors[i];
        
        // Размер вектора
        uint32_t vecSize = static_cast<uint32_t>(vec.size());
        char* vecSizePtr = reinterpret_cast<char*>(&vecSize);
        binaryData.insert(binaryData.end(), vecSizePtr, vecSizePtr + sizeof(uint32_t));
        
        std::cout << "Отладка: Вектор " << i << " имеет " << vec.size() << " элементов" << std::endl;
        
        // Значения вектора
        for (size_t j = 0; j < vec.size(); ++j) {
            double value = vec[j];
            char* valuePtr = reinterpret_cast<char*>(&value);
            binaryData.insert(binaryData.end(), valuePtr, valuePtr + sizeof(double));
        }
    }
    
    std::cout << "Отладка: Всего байт для отправки: " << binaryData.size() << std::endl;
    
    return binaryData;
}

bool DataProcessor::saveResults(const std::string& filename, const std::vector<double>& results) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        ErrorHandler::logError("Не удалось открыть файл для записи результатов: " + filename);
        return false;
    }
    
    // Количество результатов
    file << results.size() << " ";
    
    // Сами результаты
    for (size_t i = 0; i < results.size(); ++i) {
        file << results[i];
        if (i != results.size() - 1) {
            file << " ";
        }
    }
    
    file << std::endl;
    file.close();
    
    if (!file) {
        ErrorHandler::logError("Ошибка записи в файл: " + filename);
        return false;
    }
    
    return true;
}
