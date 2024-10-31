#ifndef LOG_UTILS_H
#define LOG_UTILS_H

#include <string>

std::string encryptToken(const std::string& token, const std::string& key);
std::string decryptToken(const std::string& encryptedToken, const std::string& key);
bool verifyToken(const std::string& providedToken, const std::string& encryptedToken, const std::string& key);

#endif // LOG_UTILS_H
