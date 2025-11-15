#include "client.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <openssl/sha.h>
#include <iomanip>
#include <stdexcept>

NetworkClient::NetworkClient(const std::string& ip, int port, const std::string& in_file, 
                  const std::string& out_file, const std::string& auth)
    : sock(-1), server_ip(ip), server_port(port), input_file(in_file), 
      output_file(out_file), auth_file(auth) {}

NetworkClient::~NetworkClient() {
    if (sock != -1) {
        close(sock);
    }
}

int NetworkClient::send_data(const void* data, size_t length) {
    return send(sock, data, length, 0);
}

int NetworkClient::receive_data(void* buffer, size_t length) {
    return recv(sock, buffer, length, 0);
}

uint64_t NetworkClient::htonll(uint64_t value) {
    union {
        uint32_t i;
        char c[4];
    } test = {0x01020304};
    
    if (test.c[0] == 1) {
        return value;
    } else {
        return ((uint64_t)htonl(value & 0xFFFFFFFF) << 32) | htonl(value >> 32);
    }
}

uint64_t NetworkClient::ntohll(uint64_t value) {
    return htonll(value);
}

std::string NetworkClient::sha1_hash(const std::string& input) {
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(input.c_str()), input.length(), hash);
    
    std::stringstream ss;
    for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
}

bool NetworkClient::run() {
    try {
        std::cout << "Запуск сетевого клиента..." << std::endl;
        std::cout << "Сервер: " << server_ip << ":" << server_port << std::endl;
        std::cout << "Входной файл: " << input_file << std::endl;
        std::cout << "Выходной файл: " << output_file << std::endl;
        
        connect_to_server();
        authenticate();
        
        auto vectors = read_input_file();
        send_vectors(vectors);
        auto results = receive_results(vectors.size());
        save_results(results);
        
        std::cout << "Работа завершена успешно" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        handle_error("NetworkClient::run", e.what());
        return false;
    }
}

void NetworkClient::print_help() {
    std::cout << "Использование: client <IP_СЕРВЕРА> <ВХОДНОЙ_ФАЙЛ> <ВЫХОДНОЙ_ФАЙЛ> [ОПЦИИ]" << std::endl;
    std::cout << "Опции:" << std::endl;
    std::cout << "  -p <ПОРТ>    Порт сервера (по умолчанию: 33333)" << std::endl;
    std::cout << "  -a <ФАЙЛ>    Файл аутентификации (по умолчанию: ~/.config/velient.conf)" << std::endl;
    std::cout << "  -h           Показать справку" << std::endl;
}
