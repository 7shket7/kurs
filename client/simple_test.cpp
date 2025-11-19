#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cmath>

#define TEST_CASE(name) void name()
#define REQUIRE(condition) do { \
    if (!(condition)) { \
        std::cout << "FAIL: " << #condition << " in " << __FUNCTION__ << std::endl; \
        allTestsPassed = false; \
    } else { \
        std::cout << "PASS: " << #condition << std::endl; \
    } \
} while(0)

#define REQUIRE_THROWS(expression) do { \
    bool threw = false; \
    try { expression; } \
    catch (...) { threw = true; } \
    if (!threw) { \
        std::cout << "FAIL: Expected exception in " << __FUNCTION__ << std::endl; \
        allTestsPassed = false; \
    } else { \
        std::cout << "PASS: Exception thrown as expected" << std::endl; \
    } \
} while(0)

bool allTestsPassed = true;

class TestableClient {
private:
    std::string input_file;
    std::string output_file;
    std::string auth_file;

public:
    struct VectorData {
        std::vector<double> values;
    };

    TestableClient(const std::string& in_file, const std::string& out_file, const std::string& auth = "test_auth.conf")
        : input_file(in_file), output_file(out_file), auth_file(auth) {}

    // Методы для тестирования
    std::vector<VectorData> read_input_file() {
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
        }
        
        file.close();
        return vectors;
    }

    bool save_results(const std::vector<double>& results) {
        std::ofstream file(output_file);
        if (!file.is_open()) {
            throw std::runtime_error("Не удалось создать файл результатов: " + output_file);
        }
        
        file << results.size();
        for (const auto& result : results) {
            file << " " << result;
        }
        
        file.close();
        return true;
    }

    std::string read_auth_credentials() {
        std::ifstream file(auth_file);
        if (!file.is_open()) {
            throw std::runtime_error("Не удалось открыть файл аутентификации: " + auth_file);
        }
        
        std::string login, password;
        std::getline(file, login);
        std::getline(file, password);
        
        file.close();
        
        login.erase(0, login.find_first_not_of(" \t\n\r"));
        login.erase(login.find_last_not_of(" \t\n\r") + 1);
        password.erase(0, password.find_first_not_of(" \t\n\r"));
        password.erase(password.find_last_not_of(" \t\n\r") + 1);
        
        if (login.empty() || password.empty()) {
            throw std::runtime_error("Неверный формат файла аутентификации");
        }
        
        return login + ":" + password;
    }

    std::string sha1_hash_simple(const std::string& input) {
        unsigned long hash = 5381;
        for (char c : input) {
            hash = ((hash << 5) + hash) + c; // hash * 33 + c
        }
        
        std::stringstream ss;
        ss << std::hex << hash;
        return ss.str();
    }
};

void createTestFile(const std::string& filename, const std::string& content) {
    std::ofstream file(filename);
    file << content;
    file.close();
}

bool filesEqual(const std::string& file1, const std::string& file2) {
    std::ifstream f1(file1), f2(file2);
    std::string line1, line2;
    while (std::getline(f1, line1) && std::getline(f2, line2)) {
        if (line1 != line2) return false;
    }
    return true;
}

TEST_CASE(test_read_valid_input_file) {
    createTestFile("test_input.txt", "2\n3\n1.0 2.0 3.0\n2\n4.0 5.0\n");
    
    TestableClient client("test_input.txt", "test_output.txt");
    auto vectors = client.read_input_file();
    
    REQUIRE(vectors.size() == 2);
    REQUIRE(vectors[0].values.size() == 3);
    REQUIRE(vectors[1].values.size() == 2);
    REQUIRE(std::abs(vectors[0].values[0] - 1.0) < 0.0001);
    REQUIRE(std::abs(vectors[0].values[2] - 3.0) < 0.0001);
    REQUIRE(std::abs(vectors[1].values[1] - 5.0) < 0.0001);
    
    std::remove("test_input.txt");
}

TEST_CASE(test_read_nonexistent_input_file) {
    TestableClient client("nonexistent.txt", "test_output.txt");
    REQUIRE_THROWS(client.read_input_file());
}

TEST_CASE(test_read_invalid_input_format) {
    createTestFile("invalid_input.txt", "not_a_number\n");
    
    TestableClient client("invalid_input.txt", "test_output.txt");
    REQUIRE_THROWS(client.read_input_file());
    
    std::remove("invalid_input.txt");
}

TEST_CASE(test_save_results) {
    TestableClient client("test_input.txt", "test_output.txt");
    
    std::vector<double> results = {10.5, 20.3, 30.7};
    REQUIRE(client.save_results(results));
    
    // Проверяем содержимое файла
    std::ifstream file("test_output.txt");
    std::string content;
    std::getline(file, content);
    file.close();
    
    REQUIRE(content == "3 10.5 20.3 30.7");
    std::remove("test_output.txt");
}

TEST_CASE(test_save_empty_results) {
    TestableClient client("test_input.txt", "test_output.txt");
    
    std::vector<double> results;
    REQUIRE(client.save_results(results));
    
    std::ifstream file("test_output.txt");
    std::string content;
    std::getline(file, content);
    file.close();
    
    REQUIRE(content == "0");
    std::remove("test_output.txt");
}

TEST_CASE(test_read_valid_auth_credentials) {
    createTestFile("test_auth.conf", "testuser\ntestpass\n");
    
    TestableClient client("test_input.txt", "test_output.txt", "test_auth.conf");
    auto credentials = client.read_auth_credentials();
    
    REQUIRE(credentials == "testuser:testpass");
    std::remove("test_auth.conf");
}

TEST_CASE(test_read_auth_with_spaces) {
    createTestFile("test_auth.conf", "  testuser  \n  testpass  \n");
    
    TestableClient client("test_input.txt", "test_output.txt", "test_auth.conf");
    auto credentials = client.read_auth_credentials();
    
    REQUIRE(credentials == "testuser:testpass");
    std::remove("test_auth.conf");
}

TEST_CASE(test_read_nonexistent_auth_file) {
    TestableClient client("test_input.txt", "test_output.txt", "nonexistent.conf");
    REQUIRE_THROWS(client.read_auth_credentials());
}

TEST_CASE(test_read_empty_auth_file) {
    createTestFile("empty_auth.conf", "\n\n");
    
    TestableClient client("test_input.txt", "test_output.txt", "empty_auth.conf");
    REQUIRE_THROWS(client.read_auth_credentials());
    
    std::remove("empty_auth.conf");
}

TEST_CASE(test_sha1_hash_simple) {
    TestableClient client("test_input.txt", "test_output.txt");
    
    std::string hash1 = client.sha1_hash_simple("test");
    std::string hash2 = client.sha1_hash_simple("test");
    std::string hash3 = client.sha1_hash_simple("different");
    
    REQUIRE(!hash1.empty());
    REQUIRE(hash1 == hash2); // Должны быть одинаковые для одинаковых входов
    REQUIRE(hash1 != hash3); // Должны быть разные для разных входов
}

TEST_CASE(test_vector_data_structure) {
    TestableClient::VectorData vector;
    vector.values = {1.1, 2.2, 3.3, 4.4};
    
    REQUIRE(vector.values.size() == 4);
    REQUIRE(std::abs(vector.values[0] - 1.1) < 0.0001);
    REQUIRE(std::abs(vector.values[3] - 4.4) < 0.0001);
}

TEST_CASE(test_complex_input_file) {
    createTestFile("complex_input.txt", "3\n2\n10.0 20.0\n1\n30.0\n4\n1.0 2.0 3.0 4.0\n");
    
    TestableClient client("complex_input.txt", "test_output.txt");
    auto vectors = client.read_input_file();
    
    REQUIRE(vectors.size() == 3);
    REQUIRE(vectors[0].values.size() == 2);
    REQUIRE(vectors[1].values.size() == 1);
    REQUIRE(vectors[2].values.size() == 4);
    REQUIRE(std::abs(vectors[0].values[1] - 20.0) < 0.0001);
    REQUIRE(std::abs(vectors[1].values[0] - 30.0) < 0.0001);
    REQUIRE(std::abs(vectors[2].values[3] - 4.0) < 0.0001);
    
    std::remove("complex_input.txt");
}

TEST_CASE(test_save_and_verify_large_results) {
    TestableClient client("test_input.txt", "large_output.txt");
    
    std::vector<double> results;
    for (int i = 0; i < 100; i++) {
        results.push_back(i * 1.5);
    }
    
    REQUIRE(client.save_results(results));
    
    std::ifstream file("large_output.txt");
    REQUIRE(file.good());
    
    std::string content;
    std::getline(file, content);
    file.close();
    
    std::stringstream ss(content);
    int count;
    ss >> count;
    REQUIRE(count == 100);
    
    std::remove("large_output.txt");
}

void runAllTests() {
    std::cout << "=== ЗАПУСК МОДУЛЬНЫХ ТЕСТОВ ===" << std::endl;
    
    test_read_valid_input_file();
    test_read_nonexistent_input_file();
    test_read_invalid_input_format();
    test_save_results();
    test_save_empty_results();
    test_read_valid_auth_credentials();
    test_read_auth_with_spaces();
    test_read_nonexistent_auth_file();
    test_read_empty_auth_file();
    test_sha1_hash_simple();
    test_vector_data_structure();
    test_complex_input_file();
    test_save_and_verify_large_results();
    
    std::cout << "=== РЕЗУЛЬТАТЫ ТЕСТИРОВАНИЯ ===" << std::endl;
    if (allTestsPassed) {
        std::cout << "✅ ВСЕ ТЕСТЫ ПРОЙДЕНЫ УСПЕШНО!" << std::endl;
    } else {
        std::cout << "❯ НЕКОТОРЫЕ ТЕСТЫ НЕ ПРОЙДЕНЫ!" << std::endl;
    }
}

int main() {
    std::cout << "Для запуска: ./client_test" << std::endl << std::endl;
    
    runAllTests();
    
    return allTestsPassed ? 0 : 1;
}
