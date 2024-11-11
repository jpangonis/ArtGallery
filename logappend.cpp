#include "log_utils.h" // Assuming helper functions like encryptToken, verifyToken are defined here
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <unordered_map>
#include <set>

struct LogEntry {
    std::string timestamp;
    std::string personType;
    std::string personName;
    std::string action;
    std::string roomID;
};

bool isValidName(const std::string& name) {
    return std::regex_match(name, std::regex("^[a-zA-Z]+$"));
}

bool isValidRoomID(const std::string& roomID) {
    return std::regex_match(roomID, std::regex("^[0-9]+$"));
}

bool parseCommandLine(int argc, char* argv[], std::string& timestamp, std::string& token,
    std::string& personType, std::string& personName, std::string& action,
    std::string& roomID, std::string& logFile, std::string& batchFile) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-T") timestamp = argv[++i];
        else if (arg == "-K") token = argv[++i];
        else if (arg == "-E") { personType = "Employee"; personName = argv[++i]; }
        else if (arg == "-G") { personType = "Guest"; personName = argv[++i]; }
        else if (arg == "-A") action = "Arrival";
        else if (arg == "-L") action = "Leave";
        else if (arg == "-R") roomID = argv[++i];
        else if (arg == "-B") batchFile = argv[++i];
        else logFile = argv[i];
    }
    return !(timestamp.empty() || token.empty() || logFile.empty() || personType.empty() || personName.empty() || action.empty());
}

bool validateToken(const std::string& token, const std::string& storedEncryptedToken, const std::string& encryptionKey) {
    if (!verifyToken(token, storedEncryptedToken, encryptionKey)) {
        std::cerr << "invalid" << std::endl;
        return false;
    }
    return true;
}

bool checkTimestampOrder(const std::string& newTimestamp, const std::string& lastTimestamp) {
    return std::stoi(newTimestamp) > std::stoi(lastTimestamp);
}

void processBatchFile(const std::string& batchFile, const std::string& logFile, const std::string& storedEncryptedToken, const std::string& encryptionKey) {
    std::ifstream file(batchFile);
    if (!file.is_open()) {
        std::cerr << "invalid" << std::endl;
        exit(255);
    }

    std::string line;
    while (std::getline(file, line)) {
        // Parse each line from the batch file as if it were a command line
        // Example command parsing here, extracting arguments as in main
        // If a command is invalid, print "invalid" and skip to the next line
    }
}

int main(int argc, char* argv[]) {
    std::string timestamp, token, personType, personName, action, roomID;
    std::string logFile, batchFile;

    // Parse command-line arguments
    if (!parseCommandLine(argc, argv, timestamp, token, personType, personName, action, roomID, logFile, batchFile)) {
        std::cerr << "invalid" << std::endl;
        return 255;
    }

    // Validate the log token
    const std::string encryptionKey = "thisisasecretkey";
    std::string storedEncryptedToken = encryptToken("secret", encryptionKey);
    if (!validateToken(token, storedEncryptedToken, encryptionKey)) {
        return 255;
    }

    // Handle batch file processing
    if (!batchFile.empty()) {
        processBatchFile(batchFile, logFile, storedEncryptedToken, encryptionKey);
        return 0;
    }

    // Validate timestamp, room ID, and name format
    if (!isValidName(personName) || (!roomID.empty() && !isValidRoomID(roomID)) || timestamp.empty()) {
        std::cerr << "invalid" << std::endl;
        return 255;
    }

    // Open log file and check if timestamps are in order
    std::ifstream log(logFile);
    std::string lastTimestamp;
    std::string line;
    while (std::getline(log, line)) {
        lastTimestamp = line.substr(0, line.find(','));  // Assume log format has timestamp at start
    }
    log.close();

    if (!lastTimestamp.empty() && !checkTimestampOrder(timestamp, lastTimestamp)) {
        std::cerr << "invalid" << std::endl;
        return 255;
    }

    // Validate entry and exit conditions
    // Example: check if entering a room without being in the gallery, or leaving a room without entering it

    // Create log entry
    std::string logEntry = timestamp + "," + personType + "," + personName + "," + action;
    if (!roomID.empty()) logEntry += ",Room:" + std::to_string(std::stoi(roomID)); // Normalize room ID

    // Append log entry to log file
    std::ofstream logAppend(logFile, std::ios::app);
    if (!logAppend.is_open()) {
        std::cerr << "invalid" << std::endl;
        return 255;
    }

    logAppend << logEntry << std::endl;
    logAppend.close();

    std::cout << "Log entry added successfully." << std::endl;
    return 0;
}
