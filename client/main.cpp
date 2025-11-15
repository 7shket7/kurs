#include "client.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc == 1 || std::string(argv[1]) == "-h") {
        NetworkClient::print_help();
        return 0;
    }
    
    if (argc < 4) {
        std::cerr << "Ошибка: Недостаточно аргументов" << std::endl;
        NetworkClient::print_help();
        return 1;
    }
    
    std::string server_ip = argv[1];
    std::string input_file = argv[2];
    std::string output_file = argv[3];
    int server_port = 33333;
    std::string auth_file = "~/.config/velient.conf";
    
    for (int i = 4; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-p" && i + 1 < argc) {
            server_port = std::stoi(argv[++i]);
        } else if (arg == "-a" && i + 1 < argc) {
            auth_file = argv[++i];
        } else {
            std::cerr << "Неизвестный параметр: " << arg << std::endl;
            NetworkClient::print_help();
            return 1;
        }
    }
    
    NetworkClient client(server_ip, server_port, input_file, output_file, auth_file);
    
    if (client.run()) {
        return 0;
    } else {
        return 1;
    }
}
