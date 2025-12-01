#include "Authenticator.h"
#include "ErrorHandler.h"
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>
#include <random>
#include <algorithm>
#include <cctype>

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

std::string Authenticator::hexToString(const std::string& hex) {
    std::string result;
    for (size_t i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        char byte = static_cast<char>(strtol(byteString.c_str(), nullptr, 16));
        result.push_back(byte);
    }
    return result;
}
