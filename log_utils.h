#ifndef LOG_UTILS_H
#define LOG_UTILS_H

#include <string>
#include <unordered_map>

std::string encryptData(const std::string& token, const std::string& key);
std::string decryptData(const std::string& encryptedToken, const std::string& key);
bool verifyToken(const std::string& providedToken, const std::string& encryptedToken, const std::string& key);
std::unordered_map<std::string, std::string> loadEnv(const std::string& filename);

#endif // LOG_UTILS_H
