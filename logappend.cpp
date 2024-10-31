#include "log_utils.h"
#include <iostream>
#include <fstream>
#include <string>

int main(int argc, char* argv[]) {
    std::string timestamp, token, personType, personName, action, roomID;
    std::string logFile;

    // Parse command-line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-T") timestamp = argv[++i];
        else if (arg == "-K") token = argv[++i];
        else if (arg == "-E") { personType = "Employee"; personName = argv[++i]; }
        else if (arg == "-G") { personType = "Guest"; personName = argv[++i]; }
        else if (arg == "-A") action = "Arrival";
        else if (arg == "-L") action = "Leave";
        else if (arg == "-R") roomID = argv[++i];
        else logFile = argv[i];
    }

    // Load and encrypt the stored token
    const std::string encryptionKey = "thisisasecretkey";
    std::string storedEncryptedToken = encryptToken("token123", encryptionKey);

    // Verify provided token
    if (!verifyToken(token, storedEncryptedToken, encryptionKey)) {
        std::cerr << "Invalid authentication token." << std::endl;
        return 1;
    }

    // Create the log entry in plaintext
    std::string logEntry = timestamp + "," + personType + "," + personName + "," + action;
    if (!roomID.empty()) logEntry += ",Room:" + roomID;

    // Append log entry to the log file
    std::ofstream log(logFile, std::ios::app);
    log << logEntry << std::endl;
    log.close();

    std::cout << "Log entry added successfully." << std::endl;
    return 0;
}
