#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H

#include <string>

class ErrorHandler {
public:
    static void logError(const std::string& message);
    static void exitWithError(const std::string& message);
    static void printHelp();
};

#endif // ERRORHANDLER_H
