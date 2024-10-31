//#include "log_utils.h"
//#include <iostream>
//#include <fstream>
//#include <string>
//
//int main(int argc, char* argv[]) {
//    std::string token, logFile, queryType, personType, personName;
//
//    // Parse command-line arguments
//    for (int i = 1; i < argc; i++) {
//        std::string arg = argv[i];
//        if (arg == "-K") token = argv[++i];
//        else if (arg == "-S") queryType = "State";
//        else if (arg == "-R") queryType = "Person";
//        else if (arg == "-E") { personType = "Employee"; personName = argv[++i]; }
//        else if (arg == "-G") { personType = "Guest"; personName = argv[++i]; }
//        else logFile = argv[i];
//    }
//
//    // Load and encrypt the stored token
//    const std::string encryptionKey = "thisisaverysecret!";
//    std::string storedEncryptedToken = encryptToken("token123", encryptionKey);
//
//    // Verify provided token
//    if (!verifyToken(token, storedEncryptedToken, encryptionKey)) {
//        std::cerr << "Invalid authentication token." << std::endl;
//        return 1;
//    }
//
//    // Read and display log entries based on the query
//    std::ifstream log(logFile);
//    std::string logEntry;
//    while (std::getline(log, logEntry)) {
//        if (queryType == "State") {
//            std::cout << logEntry << std::endl;
//        }
//        else if (queryType == "Person" && logEntry.find(personName) != std::string::npos) {
//            std::cout << logEntry << std::endl;
//        }
//    }
//    log.close();
//
//    return 0;
//}
