#include "log_utils.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <unordered_map>
#include <set>
#include <sstream>

// Structure to represent a log entry
struct LogEntry {
    std::string timestamp;
    std::string personType;
    std::string personName;
    std::string action;
    std::string roomID;
};

// Maps to maintain the current state of the system
std::unordered_map<std::string, std::set<std::string>> personRoomMap;
std::unordered_map<std::string, bool> inGalleryMap;

// Validate the name using regex
bool isValidName(const std::string& name) {
    return std::regex_match(name, std::regex("^[a-zA-Z]+$"));
}

// Validate the room ID using regex
bool isValidRoomID(const std::string& roomID) {
    return std::regex_match(roomID, std::regex("^[0-9]+$"));
}

// Parse the command-line arguments
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

// Validate the token
bool validateToken(const std::string& token, const std::string& storedEncryptedToken, const std::string& encryptionKey) {
    if (!verifyToken(token, storedEncryptedToken, encryptionKey)) {
        std::cerr << "invalid" << std::endl;
        return false;
    }
    return true;
}

// Check if a new timestamp is greater than the last timestamp
bool checkTimestampOrder(const std::string& newTimestamp, const std::string& lastTimestamp) {
    return std::stoi(newTimestamp) > std::stoi(lastTimestamp);
}

// Validate state consistency for departure
bool validateStateForDeparture(const std::string& personName, const std::string& roomID, bool leavingGallery) {
    if (leavingGallery) {
        if (!personRoomMap[personName].empty()) {
            return false;
        }
        return inGalleryMap[personName];
    }
    else {
        return personRoomMap[personName].count(roomID) > 0;
    }
}

// Prevent entering a room before entering the gallery
bool validateStateForArrival(const std::string& personName, const std::string& roomID) {
    if (roomID.empty()) {
        return true; // No room specified, so it's an arrival at the gallery.
    }
    // If a room is specified, check if the person has already entered the gallery.
    return inGalleryMap[personName];
}

// Update the state of the system based on the action
void updateState(const std::string& personName, const std::string& roomID, const std::string& action) {
    if (action == "Arrival") {
        // Ensure the person enters the gallery first, before entering any room
        if (!validateStateForArrival(personName, roomID)) {
            std::cerr << "invalid" << std::endl;
            exit(255);  // Exit immediately if inconsistency is found
        }

        inGalleryMap[personName] = true;
        if (!roomID.empty()) {
            personRoomMap[personName].insert(roomID);
        }
    }
    else if (action == "Leave") {
        if (roomID.empty()) {
            personRoomMap[personName].clear();
            inGalleryMap[personName] = false;
        }
        else {
            personRoomMap[personName].erase(roomID);
        }
    }
}

// Process the log file to rebuild the current state
bool processLogFile(const std::string& logFile, const std::string& encryptionKey, std::string& lastTimestamp) {
    std::fstream log(logFile, std::ios::in | std::ios::out | std::ios::app); // Open for reading and appending
    if (!log.is_open()) {
        std::cerr << "Error: Unable to open or create log file: " << logFile << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(log, line)) {
        size_t pos = line.find(',');
        if (pos != std::string::npos) {
            lastTimestamp = line.substr(0, pos); // Extract the timestamp
        }

        std::string decryptedLogEntry = decryptData(line, encryptionKey);
        std::vector<std::string> tokens;
        std::istringstream ss(decryptedLogEntry);
        std::string token;
        while (std::getline(ss, token, ',')) {
            tokens.push_back(token);
        }

        if (tokens.size() >= 4) {
            std::string personName = tokens[2];
            std::string action = tokens[3];
            std::string roomID = tokens.size() > 4 ? tokens[4].substr(5) : "";
            updateState(personName, roomID, action);
        }
    }
    log.close();
    return true;
}

// Main function
int main(int argc, char* argv[]) {
    std::string timestamp, token, personType, personName, action, roomID;
    std::string logFile, batchFile;

    auto envVars = loadEnv(".env");
    std::string encryptionKey = envVars["ENCRYPTION_KEY"];
    std::string secret = envVars["SECRET"];

    if (!parseCommandLine(argc, argv, timestamp, token, personType, personName, action, roomID, logFile, batchFile)) {
        std::cerr << "invalid" << std::endl;
        return 255;
    }

    std::string storedEncryptedToken = encryptData(secret, encryptionKey);
    if (!validateToken(token, storedEncryptedToken, encryptionKey)) {
        return 255;
    }

    if (!isValidName(personName) || (!roomID.empty() && !isValidRoomID(roomID)) || timestamp.empty()) {
        std::cerr << "invalid" << std::endl;
        return 255;
    }

    std::string lastTimestamp;
    if (!processLogFile(logFile, encryptionKey, lastTimestamp)) {
        std::cerr << "invalid" << std::endl;
        return 255;
    }

    if (!lastTimestamp.empty() && !checkTimestampOrder(timestamp, lastTimestamp)) {
        std::cerr << "invalid" << std::endl;
        return 255;
    }

    if (action == "Leave") {
        bool leavingGallery = roomID.empty();
        if (!validateStateForDeparture(personName, roomID, leavingGallery)) {
            std::cerr << "invalid" << std::endl;
            return 255;
        }
    }

    updateState(personName, roomID, action);

    std::string logEntry = timestamp + "," + personType + "," + personName + "," + action;
    if (!roomID.empty()) logEntry += ",Room:" + roomID;

    std::string encryptedLogEntry = encryptData(logEntry, encryptionKey);

    std::ofstream logAppend(logFile, std::ios::app);
    if (!logAppend.is_open()) {
        std::cerr << "invalid" << std::endl;
        return 255;
    }

    logAppend << encryptedLogEntry << std::endl;
    logAppend.close();

    std::cout << "Log entry added successfully." << std::endl;
    return 0;
}
