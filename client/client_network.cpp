#include "client.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <arpa/inet.h>
#include <sstream>
#include <cstring>

std::vector<NetworkClient::VectorData> NetworkClient::read_input_file() {
    std::ifstream file(input_file);
    if (!file.is_open()) {
        throw std::runtime_error("Не удалось открыть входной файл: " + input_file);
    }
    
    std::vector<VectorData> vectors;
    std::string line;
    
    uint32_t vector_count;
    if (!std::getline(file, line)) {
        throw std::runtime_error("Ошибка чтения количества векторов");
    }
    
    std::stringstream ss(line);
    if (!(ss >> vector_count)) {
        throw std::runtime_error("Неверный формат количества векторов");
    }
    
    std::cout << "Векторов для обработки: " << vector_count << std::endl;
    
    for (uint32_t i = 0; i < vector_count; ++i) {
        if (!std::getline(file, line)) {
            throw std::runtime_error("Ошибка чтения размера вектора " + std::to_string(i + 1));
        }
        
        ss.clear();
        ss.str(line);
        uint32_t vector_size;
        if (!(ss >> vector_size)) {
            throw std::runtime_error("Неверный формат размера вектора " + std::to_string(i + 1));
        }
        
        if (!std::getline(file, line)) {
            throw std::runtime_error("Ошибка чтения значений вектора " + std::to_string(i + 1));
        }
        
        ss.clear();
        ss.str(line);
        VectorData vector;
        vector.values.resize(vector_size);
        
        for (uint32_t j = 0; j < vector_size; ++j) {
            double value;
            if (!(ss >> value)) {
                throw std::runtime_error("Ошибка чтения значения " + std::to_string(j + 1) + 
                                       " вектора " + std::to_string(i + 1));
            }
            vector.values[j] = value;
        }
        
        vectors.push_back(vector);
        std::cout << "Вектор " << i + 1 << ": размер=" << vector_size << std::endl;
    }
    
    file.close();
    return vectors;
}

bool NetworkClient::connect_to_server() {
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        throw std::runtime_error("Ошибка создания сокета");
    }
    
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(server_port);
    
    if (inet_pton(AF_INET, server_ip.c_str(), &address.sin_addr) <= 0) {
        throw std::runtime_error("Неверный адрес: " + server_ip);
    }
    
    if (connect(sock, (sockaddr*)&address, sizeof(address)) < 0) {
        throw std::runtime_error("Ошибка соединения с " + server_ip + ":" + std::to_string(server_port));
    }
    
    std::cout << "Успешное соединение с сервером " << server_ip << ":" << server_port << std::endl;
    return true;
}

bool NetworkClient::send_vectors(const std::vector<VectorData>& vectors) {
    // Отправка количества векторов (БЕЗ преобразования порядка байт)
    uint32_t vector_count = vectors.size();
    std::cout << "Отправка количества векторов: " << vector_count << " (байт: " << sizeof(vector_count) << ")" << std::endl;
    if (send_data(&vector_count, sizeof(vector_count)) <= 0) {
        throw std::runtime_error("Ошибка отправки количества векторов");
    }
    
    std::cout << "Отправка " << vectors.size() << " векторов на сервер..." << std::endl;
    
    for (size_t i = 0; i < vectors.size(); ++i) {
        const auto& vector = vectors[i];
        
        // Отправка размера вектора (БЕЗ преобразования порядка байт)
        uint32_t vector_size = vector.values.size();
        std::cout << "Отправка размера вектора " << i + 1 << ": " << vector_size << " (байт: " << sizeof(vector_size) << ")" << std::endl;
        if (send_data(&vector_size, sizeof(vector_size)) <= 0) {
            throw std::runtime_error("Ошибка отправки размера вектора " + std::to_string(i + 1));
        }
        
        // Отправка значений вектора (БЕЗ преобразования порядка байт)
        std::cout << "Отправка значений вектора " << i + 1 << " (байт: " << vector.values.size() * sizeof(double) << ")" << std::endl;
        if (send_data(vector.values.data(), vector.values.size() * sizeof(double)) <= 0) {
            throw std::runtime_error("Ошибка отправки значений вектора " + std::to_string(i + 1));
        }
        
        std::cout << "Вектор " << i + 1 << " отправлен (размер: " << vector.values.size() << ")" << std::endl;
    }
    
    return true;
}

std::vector<double> NetworkClient::receive_results(uint32_t expected_count) {
    std::vector<double> results;
    
    std::cout << "Ожидание " << expected_count << " результатов от сервера..." << std::endl;
    
    for (uint32_t i = 0; i < expected_count; ++i) {
        double result;
        int bytes_received = receive_data(&result, sizeof(result));
        
        if (bytes_received <= 0) {
            throw std::runtime_error("Ошибка получения результата " + std::to_string(i + 1));
        }
        
        results.push_back(result);
        std::cout << "Получен результат " << i + 1 << ": " << result << std::endl;
    }
    
    return results;
}

bool NetworkClient::save_results(const std::vector<double>& results) {
    std::ofstream file(output_file);
    if (!file.is_open()) {
        throw std::runtime_error("Не удалось создать файл результатов: " + output_file);
    }
    
    file << results.size();
    for (const auto& result : results) {
        file << " " << result;
    }
    
    file.close();
    std::cout << "Результаты сохранены в: " << output_file << std::endl;
    std::cout << "Количество результатов: " << results.size() << std::endl;
    
    return true;
}

void NetworkClient::handle_error(const std::string& function_name, const std::string& error_details) {
    std::cerr << "ОШИБКА:" << std::endl;
    std::cerr << "Функция: " << function_name << std::endl;
    std::cerr << "Параметры: IP=" << server_ip << ", PORT=" << server_port 
              << ", INPUT=" << input_file << ", OUTPUT=" << output_file << std::endl;
    std::cerr << "Ошибка: " << error_details << std::endl;
    std::cerr << "Программа завершена." << std::endl;
}
