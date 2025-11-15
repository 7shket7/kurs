#ifndef CLIENT_H
#define CLIENT_H

#include <vector>
#include <string>
#include <cstdint>

class NetworkClient {
private:
    int sock;
    std::string server_ip;
    int server_port;
    std::string input_file;
    std::string output_file;
    std::string auth_file;
    
    struct VectorData {
        std::vector<double> values;
    };

public:
    NetworkClient(const std::string& ip, int port, const std::string& in_file, 
                  const std::string& out_file, const std::string& auth = "~/.config/velient.conf");
    ~NetworkClient();
    
    bool run();
    static void print_help();

private:
    std::string sha1_hash(const std::string& input);
    std::string read_auth_credentials();
    bool connect_to_server();
    bool authenticate();
    std::vector<VectorData> read_input_file();
    bool send_vectors(const std::vector<VectorData>& vectors);
    std::vector<double> receive_results(uint32_t expected_count);
    bool save_results(const std::vector<double>& results);
    void handle_error(const std::string& function_name, const std::string& error_details);
    
    int send_data(const void* data, size_t length);
    int receive_data(void* buffer, size_t length);
    
    uint64_t htonll(uint64_t value);
    uint64_t ntohll(uint64_t value);
};

#endif
