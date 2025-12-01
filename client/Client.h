#ifndef CLIENT_H
#define CLIENT_H

#include <string>

struct ClientConfig {
    std::string serverAddress;
    int serverPort;
    std::string inputFileName;
    std::string outputFileName;
    std::string configFileName;
    std::string login;
    std::string password;
    
    ClientConfig();
};

class Client {
private:
    ClientConfig config;
    
    bool parseCommandLineArgs(int argc, char* argv[]);
    bool readConfigFile();
    
public:
    bool run(int argc, char* argv[]);
};

#endif // CLIENT_H
