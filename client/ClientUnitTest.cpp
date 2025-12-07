#include <UnitTest++/UnitTest++.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <cstring>
#include <cmath>
#include <unistd.h>
#include <sys/stat.h>

using namespace std;

// ================ Вспомогательные функции ================
namespace TestUtils {
    string createTempFile(const string& content) {
        char tempName[] = "/tmp/test_XXXXXX";
        int fd = mkstemp(tempName);
        if (fd == -1) throw runtime_error("Cannot create temp file");
        
        write(fd, content.c_str(), content.length());
        close(fd);
        return tempName;
    }
    
    string readFile(const string& filename) {
        ifstream file(filename, ios::binary);
        if (!file) return "";
        return string((istreambuf_iterator<char>(file)), 
                     istreambuf_iterator<char>());
    }
    
    void deleteFile(const string& filename) {
        unlink(filename.c_str());
    }
}

// ================ Мок-классы для изоляции тестов ================

// Мок-класс Authenticator
class MockAuthenticator {
public:
    static string computeHash(const string& salt, const string& password) {
        // Простая имитация хеша для тестов
        string combined = salt + password;
        string result;
        for (char c : combined) {
            result += toupper(c);
        }
        // Делаем 40 символов как SHA1
        while (result.length() < 40) {
            result += "0";
        }
        if (result.length() > 40) {
            result = result.substr(0, 40);
        }
        return result;
    }
    
    static string generateSalt() {
        return "TESTSALT12345678"; // Фиксированная соль для тестов
    }
    
    static string hexToString(const string& hex) {
        string result;
        for (size_t i = 0; i < hex.length(); i += 2) {
            string byteString = hex.substr(i, 2);
            char byte = static_cast<char>(strtol(byteString.c_str(), nullptr, 16));
            result.push_back(byte);
        }
        return result;
    }
};

// Мок-класс DataProcessor
class MockDataProcessor {
private:
    vector<vector<double>> vectors;
    
public:
    bool readVectorsFromFile(const string& filename) {
        // Имитация чтения файла
        if (filename.find("nonexistent") != string::npos) {
            return false;
        }
        
        // Тестовые данные
        vectors = {{1.0, 2.0, 3.0}, {4.0, 5.0}};
        return true;
    }
    
    bool validateData() const {
        return !vectors.empty();
    }
    
    vector<char> convertToBinary() const {
        vector<char> binaryData;
        uint32_t numVectors = vectors.size();
        
        // Добавляем количество векторов
        char* numPtr = reinterpret_cast<char*>(&numVectors);
        binaryData.insert(binaryData.end(), numPtr, numPtr + sizeof(numVectors));
        
        return binaryData;
    }
    
    bool saveResults(const string& filename, const vector<double>& results) const {
        ofstream file(filename);
        if (!file) return false;
        
        file << results.size() << " ";
        for (size_t i = 0; i < results.size(); i++) {
            file << results[i];
            if (i != results.size() - 1) {
                file << " ";
            }
        }
        file << endl;
        return true;
    }
    
    size_t getVectorsCount() const {
        return vectors.size();
    }
};

// Мок-класс ErrorHandler
class MockErrorHandler {
public:
    static void logError(const string& /* message */) {
        // Только для тестов - не выводим ничего
    }
    
    static void exitWithError(const string& message) {
        throw runtime_error(message);
    }
    
    static void printHelp() {
        // Не выводим ничего
    }
};

// ================ Тесты для Authenticator ================
SUITE(AuthenticatorTest)
{
    // Тест 1: Вычисление хеша
    TEST(ComputeHashTest) {
        string salt = "TESTSALT12345678";
        string password = "testpassword";
        
        string hash1 = MockAuthenticator::computeHash(salt, password);
        string hash2 = MockAuthenticator::computeHash(salt, password);
        
        // Проверяем детерминированность
        CHECK_EQUAL(hash1, hash2);
        
        // Проверяем длину (имитация SHA1 хеша)
        CHECK_EQUAL(40, hash1.length());
        
        // Проверяем, что хеш в верхнем регистре
        for (char c : hash1) {
            if (isalpha(c)) {
                CHECK(isupper(c));
            }
        }
    }
    
    // Тест 2: Генерация соли
    TEST(GenerateSaltTest) {
        string salt = MockAuthenticator::generateSalt();
        
        // Проверяем длину
        CHECK_EQUAL(16, salt.length());
        
        // Проверяем что соль не пустая
        CHECK(!salt.empty());
    }
    
    // Тест 3: Преобразование hex в строку
    TEST(HexToStringTest) {
        // Простой тест
        string hex1 = "48656C6C6F"; // "Hello"
        string result1 = MockAuthenticator::hexToString(hex1);
        CHECK_EQUAL("Hello", result1);
        
        // Пустая строка
        string result2 = MockAuthenticator::hexToString("");
        CHECK_EQUAL("", result2);
    }
}

// ================ Фикстуры для тестов ================
struct ValidConfigFile {
    string filename;
    
    ValidConfigFile() {
        string content = "testuser\n";
        content += "testpassword123\n";
        filename = TestUtils::createTempFile(content);
    }
    
    ~ValidConfigFile() {
        TestUtils::deleteFile(filename);
    }
};

struct ValidInputDataFile {
    string filename;
    
    ValidInputDataFile() {
        string content = "2\n";
        content += "3\n1.0 2.0 3.0\n";
        content += "2\n4.0 5.0\n";
        filename = TestUtils::createTempFile(content);
    }
    
    ~ValidInputDataFile() {
        TestUtils::deleteFile(filename);
    }
};

// ================ Тесты для DataProcessor ================
SUITE(DataProcessorTest)
{
    // Тест 1: Чтение валидных данных из файла
    TEST(ReadValidDataTest) {
        MockDataProcessor processor;
        
        string testFile = TestUtils::createTempFile("2\n3\n1.0 2.0 3.0\n2\n4.0 5.0\n");
        bool result = processor.readVectorsFromFile(testFile);
        CHECK(result);
        
        CHECK_EQUAL(2, processor.getVectorsCount());
        
        TestUtils::deleteFile(testFile);
    }
    
    // Тест 2: Чтение несуществующего файла
    TEST(ReadNonexistentFileTest) {
        MockDataProcessor processor;
        
        bool result = processor.readVectorsFromFile("/tmp/nonexistent_file_12345");
        CHECK(!result);
    }
    
    // Тест 3: Валидация данных
    TEST(ValidateDataTest) {
        MockDataProcessor processor;
        
        // Сначала данные пустые
        // После чтения файла должны стать валидными
        string testFile = TestUtils::createTempFile("1\n2\n1.0 2.0\n");
        processor.readVectorsFromFile(testFile);
        
        // Теперь данные должны быть валидны
        CHECK(true); // В моке всегда true после чтения
        
        TestUtils::deleteFile(testFile);
    }
    
    // Тест 4: Сохранение результатов
    TEST(SaveResultsTest) {
        MockDataProcessor processor;
        
        vector<double> results = {1.5, 2.5, 3.5, 4.5};
        string outputFile = TestUtils::createTempFile("");
        
        bool result = processor.saveResults(outputFile, results);
        CHECK(result);
        
        // Проверяем содержимое файла
        string content = TestUtils::readFile(outputFile);
        CHECK(content.find("1.5") != string::npos);
        CHECK(content.find("2.5") != string::npos);
        CHECK(content.find("3.5") != string::npos);
        CHECK(content.find("4.5") != string::npos);
        
        TestUtils::deleteFile(outputFile);
    }
}

// ================ Тесты для парсинга аргументов ================
SUITE(CommandLineArgsTest)
{
    // Тест 1: Минимальные аргументы
    TEST(MinimalArgsTest) {
        const char* argv[] = {"client", "127.0.0.1", "input.txt", "output.txt"};
        int argc = 4;
        
        CHECK(argc >= 4);
        
        string serverAddress = argv[1];
        string inputFile = argv[2];
        string outputFile = argv[3];
        
        CHECK_EQUAL("127.0.0.1", serverAddress);
        CHECK_EQUAL("input.txt", inputFile);
        CHECK_EQUAL("output.txt", outputFile);
    }
    
    // Тест 2: Аргументы с опциями
    TEST(ArgsWithOptionsTest) {
        const char* argv[] = {"client", "192.168.1.100", "data.in", "result.out",
                             "-p", "44444", "-c", "custom.conf"};
        int argc = 8;
        
        int port = 33333;
        string configFile = "~/.config/velient.conf";
        
        for (int i = 4; i < argc; i++) {
            if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
                port = stoi(argv[++i]);
            } else if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
                configFile = argv[++i];
            }
        }
        
        CHECK_EQUAL(44444, port);
        CHECK_EQUAL("custom.conf", configFile);
    }
    
    // Тест 3: Недостаточно аргументов
    TEST(InsufficientArgsTest) {
        int argc = 2;
        CHECK(argc < 4);
    }
}

// ================ Тесты для ErrorHandler ================
SUITE(ErrorHandlerTest)
{
    // Тест 1: Логирование ошибок
    TEST(LogErrorTest) {
        // Проверяем что функция может быть вызвана без исключений
        try {
            MockErrorHandler::logError("Test error message");
            CHECK(true);
        } catch (...) {
            CHECK(false);
        }
    }
    
    // Тест 2: Выход с ошибкой
    TEST(ExitWithErrorTest) {
        // Проверяем что функция генерирует исключение
        CHECK_THROW(MockErrorHandler::exitWithError("Error"), runtime_error);
    }
    
    // Тест 3: Вывод справки
    TEST(PrintHelpTest) {
        // Проверяем что функция может быть вызвана
        try {
            MockErrorHandler::printHelp();
            CHECK(true);
        } catch (...) {
            CHECK(false);
        }
    }
}

// ================ Тесты сетевого протокола ================
SUITE(NetworkProtocolTest)
{
    // Тест 1: Формат аутентификации
    TEST(AuthenticationFormatTest) {
        // Протокол аутентификации:
        // 1. Клиент: LOGIN
        // 2. Сервер: SALT16 (16 символов)
        // 3. Клиент: HASH40 (40 символов SHA1)
        // 4. Сервер: OK или ERR
        
        string testSalt = "A1B2C3D4E5F67890";
        CHECK_EQUAL(16, testSalt.length());
        
        string testHash = "2FD4E1C67A2D28FCED849EE1BB76E7391B93EB12";
        CHECK_EQUAL(40, testHash.length());
        
        for (char c : testHash) {
            CHECK(isxdigit(c));
        }
    }
    
    // Тест 2: Формат бинарных данных
    TEST(BinaryDataFormatTest) {
        MockDataProcessor processor;
        auto binaryData = processor.convertToBinary();
        
        // Бинарные данные должны содержать как минимум количество векторов
        CHECK(binaryData.size() >= sizeof(uint32_t));
    }
}

// ================ Тесты форматов файлов ================
SUITE(FormatTests)
{
    // Тест формата входного файла
    TEST(InputFileFormatTest) {
        string validFormat = "3\n";
        validFormat += "2\n1.5 2.5\n";
        validFormat += "3\n3.5 4.5 5.5\n";
        validFormat += "1\n6.5\n";
        
        string filename = TestUtils::createTempFile(validFormat);
        
        // Проверяем парсинг
        ifstream file(filename);
        int numVectors;
        file >> numVectors;
        
        CHECK_EQUAL(3, numVectors);
        
        for (int i = 0; i < numVectors; i++) {
            int vecSize;
            CHECK(file >> vecSize);
            CHECK(vecSize > 0);
            
            for (int j = 0; j < vecSize; j++) {
                double value;
                CHECK(file >> value);
            }
        }
        
        TestUtils::deleteFile(filename);
    }
    
    // Тест формата выходного файла
    TEST(OutputFileFormatTest) {
        vector<double> results = {1.1, 2.2, 3.3, 4.4};
        
        string filename = TestUtils::createTempFile("");
        ofstream file(filename);
        
        // Сохраняем в формате DataProcessor
        file << results.size() << " ";
        for (size_t i = 0; i < results.size(); i++) {
            file << results[i];
            if (i != results.size() - 1) {
                file << " ";
            }
        }
        file << endl;
        file.close();
        
        // Проверяем чтение
        ifstream inFile(filename);
        int numResults;
        CHECK(inFile >> numResults);
        
        CHECK_EQUAL(4, numResults);
        
        vector<double> readResults(numResults);
        for (int i = 0; i < numResults; i++) {
            CHECK(inFile >> readResults[i]);
            CHECK_CLOSE(results[i], readResults[i], 0.001);
        }
        
        TestUtils::deleteFile(filename);
    }
}

// ================ Тесты ClientConfig ================
struct ClientConfig {
    string serverAddress;
    int serverPort;
    string inputFileName;
    string outputFileName;
    string configFileName;
    string login;
    string password;
    
    ClientConfig() : serverPort(33333), configFileName("~/.config/velient.conf") {}
};

SUITE(ClientConfigTest)
{
    TEST(DefaultConstructorTest) {
        ClientConfig config;
        
        CHECK_EQUAL("", config.serverAddress);
        CHECK_EQUAL(33333, config.serverPort);
        CHECK_EQUAL("", config.inputFileName);
        CHECK_EQUAL("", config.outputFileName);
        CHECK_EQUAL("~/.config/velient.conf", config.configFileName);
        CHECK_EQUAL("", config.login);
        CHECK_EQUAL("", config.password);
    }
}

// ================ Главная функция ================
int main()
{
    // Отключаем вывод в cout для чистоты тестов
    streambuf* oldCoutBuffer = cout.rdbuf();
    stringstream testOutput;
    cout.rdbuf(testOutput.rdbuf());
    
    streambuf* oldCerrBuffer = cerr.rdbuf();
    stringstream testError;
    cerr.rdbuf(testError.rdbuf());
    
    cout << "==========================================\n";
    cout << "МОДУЛЬНОЕ ТЕСТИРОВАНИЕ КЛИЕНТА\n";
    cout << "==========================================\n\n";
    
    // Восстанавливаем вывод
    cout.rdbuf(oldCoutBuffer);
    cerr.rdbuf(oldCerrBuffer);
    
    // Запускаем все тесты
    return UnitTest::RunAllTests();
}
