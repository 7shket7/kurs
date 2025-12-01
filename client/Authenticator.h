#ifndef AUTHENTICATOR_H
#define AUTHENTICATOR_H

#include <string>
#include <vector>

class Authenticator {
public:
    static std::string computeHash(const std::string& salt, const std::string& password);
    static std::string generateSalt();
    static std::string hexToString(const std::string& hex);
};

#endif // AUTHENTICATOR_H
