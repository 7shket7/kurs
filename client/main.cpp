#include "Client.h"
#include "ErrorHandler.h"

int main(int argc, char* argv[]) {
    if (argc == 1) {
        ErrorHandler::printHelp();
        return EXIT_SUCCESS;
    }
    
    Client client;
    if (!client.run(argc, argv)) {
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
