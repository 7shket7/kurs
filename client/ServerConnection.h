#ifndef SERVERCONNECTION_H
#define SERVERCONNECTION_H

#include <string>
#include <vector>

class ServerConnection {
private:
    int socketFD;
    std::string login;
    std::string password;
    
    bool sendText(const std::string& text);
    bool receiveText(std::string& text);
    bool sendBinaryData(const void* data, size_t size);
    bool receiveBinaryData(void* data, size_t size);
    
public:
    ServerConnection();
    ~ServerConnection();
    
    bool establishConnection(const std::string& address, int port);
    bool authenticate(const std::string& login, const std::string& password);
    bool sendVectors(const std::vector<std::vector<double>>& vectors, std::vector<double>& results);
    void closeConnection();
};

#endif // SERVERCONNECTION_H
