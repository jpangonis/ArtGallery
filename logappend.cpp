#include "log_utils.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <unordered_map>
#include <set>
#include <sstream>

using std::cout; using std::endl; using std::string; using std::vector; using std::cerr;
using std::ifstream; using std::unordered_map; using std::set; using std::getline; using std::ofstream;
using std::stoi;  using std::regex_match; using std::regex; using std::ios; using std::istringstream; 
using std::fstream;

// Structure to represent a log entry
struct LogEntry {
    string timestamp;
    string personType;
    string personName;
    string action;
    string roomID;
};

// Maps to maintain the current state of the system
unordered_map<string, set<string>> personRoomMap;
unordered_map<string, bool> inGalleryMap;

// Validate the name using regex
bool isValidName(const string& name) {
    return regex_match(name, regex("^[a-zA-Z]+$"));
}

// Validate the room ID using regex
bool isValidRoomID(const string& roomID) {
    return regex_match(roomID, regex("^[0-9]+$"));
}

// Parse the command-line arguments
bool parseCommandLine(int argc, char* argv[], string& timestamp, string& token,
    string& personType, string& personName, string& action,
    string& roomID, string& logFile, string& batchFile) {
    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
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
bool validateToken(const string& token, const string& storedEncryptedToken, const string& encryptionKey) {
    if (!verifyToken(token, storedEncryptedToken, encryptionKey)) {
        cerr << "invalid" << endl;
        return false;
    }
    return true;
}

// Check if a new timestamp is greater than the last timestamp
bool checkTimestampOrder(const string& newTimestamp, const string& lastTimestamp) {
    if (lastTimestamp.empty())
        return true;
    return stoi(newTimestamp) > stoi(lastTimestamp);
}

// Validate state consistency for departure
bool validateStateForDeparture(const string& personName, const string& roomID, bool leavingGallery) {
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
bool validateStateForArrival(const string& personName, const string& roomID) {
    if (roomID.empty()) {
        return true; // No room specified, so it's an arrival at the gallery.
    }
    // If a room is specified, check if the person has already entered the gallery.
    return inGalleryMap[personName];
}

// Update the state of the system based on the action
void updateState(const string& personName, const string& roomID, const string& action) {
    if (action == "Arrival") {
        // Ensure the person enters the gallery first, before entering any room
        if (!validateStateForArrival(personName, roomID)) {
            cerr << "invalid" << endl;
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
bool processLogFile(const string& logFile, const string& encryptionKey, string& lastTimestamp) {
    fstream log(logFile, ios::in | ios::out | ios::app); // Open for reading and appending

    // Check if the log file exists, if not, create it
    if (!log.is_open()) {
        ofstream createLog(logFile);  // Create an empty log file if it doesn't exist
        if (!createLog.is_open()) {
            return false;
        }
        createLog.close();  // Close the file after creation
        log.open(logFile, ios::in | ios::out | ios::app);  // Reopen in read-write mode
    }

    if (!log.is_open()) {
        return false;
    }

    string line;
    while (getline(log, line)) {
        string decryptedLogEntry = decryptData(line, encryptionKey);
        size_t pos = decryptedLogEntry.find(',');
        if (pos != string::npos) {
            lastTimestamp = decryptedLogEntry.substr(0, pos); // Extract the timestamp from the decrypted entry
        }

        vector<string> tokens;
        istringstream ss(decryptedLogEntry);
        string token;
        while (getline(ss, token, ',')) {
            tokens.push_back(token);
        }

        if (tokens.size() >= 4) {
            string personName = tokens[2];
            string action = tokens[3];
            string roomID = tokens.size() > 4 ? tokens[4].substr(5) : "";
            updateState(personName, roomID, action);
        }
    }

    log.close();
    return true;
}


// Main function
int main(int argc, char* argv[]) {
    string timestamp, token, personType, personName, action, roomID;
    string logFile, batchFile;

    //load secrets
    auto envVars = loadEnv(".env");
    string encryptionKey = envVars["ENCRYPTION_KEY"];
    string secret = envVars["SECRET"];

    //parse command line
    if (!parseCommandLine(argc, argv, timestamp, token, personType, personName, action, roomID, logFile, batchFile)) {
        cerr << "invalid" << endl;
        return 255;
    }

    //validate token
    string storedEncryptedToken = encryptData(secret, encryptionKey);
    if (!validateToken(token, storedEncryptedToken, encryptionKey)) {
        return 255;
    }

    //validate user input
    if (!isValidName(personName) || (!roomID.empty() && !isValidRoomID(roomID)) || timestamp.empty()) {
        cerr << "invalid" << endl;
        return 255;
    }
        
    //open log file, decrypt it, find the last timestamp
    string lastTimestamp;
    if (!processLogFile(logFile, encryptionKey, lastTimestamp)) {
        cerr << "invalid" << endl;
        return 255;
    }

    //validate timestamp
    if (!checkTimestampOrder(timestamp, lastTimestamp)) {
        cerr << "invalid" << endl;
        return 255;
    }

    //validate person is in gallery/room
    if (action == "Leave") {
        bool leavingGallery = roomID.empty();
        if (!validateStateForDeparture(personName, roomID, leavingGallery)) {
            cerr << "invalid" << endl;
            return 255;
        }
    }

    updateState(personName, roomID, action);

    string logEntry = timestamp + "," + personType + "," + personName + "," + action;
    if (!roomID.empty()) logEntry += ",Room:" + roomID;

    string encryptedLogEntry = encryptData(logEntry, encryptionKey);

    ofstream logAppend(logFile, ios::app);
    if (!logAppend.is_open()) {
        cerr << "invalid" << endl;
        return 255;
    }

    logAppend << encryptedLogEntry << endl;
    logAppend.close();

    cout << "Log entry added successfully." << endl;
    return 0;
}
