#include "log_utils.h" 
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <sstream>

using std::cout; using std::endl; using std::string; using std::vector; using std::cerr;
using std::ifstream; using std::map; using std::set; using std::getline; using std::istringstream;
using std::stoi; using std::invalid_argument; using std::out_of_range;

int main(int argc, char* argv[]) {
    string token, logFile, queryType, personType, personName;
    vector<string> persons;
    bool multiplePersons = false;

    // Parse command-line arguments
    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        if (arg == "-K") token = argv[++i];
        else if (arg == "-S") queryType = "State";
        else if (arg == "-R") queryType = "RoomList";
        //else if (arg == "-T") queryType = "TotalTime"; //not implemented
        //else if (arg == "-I") { queryType = "SharedRooms"; multiplePersons = true; } //not implemented
        else if (arg == "-E") { personType = "Employee"; persons.push_back(argv[++i]); }
        else if (arg == "-G") { personType = "Guest"; persons.push_back(argv[++i]); }
        else logFile = argv[i];
    }

    // Check if only one query type is specified
    if (queryType.empty() || logFile.empty() || token.empty()) {
        cerr << "invalid" << endl;
        return 255;
    }

    auto envVars = loadEnv(".env");

    string encryptionKey = envVars["ENCRYPTION_KEY"];
    string secret = envVars["SECRET"];

    // Load and encrypt the stored token for verification
    string storedEncryptedToken = encryptData(secret, encryptionKey);

    // Verify provided token
    if (!verifyToken(token, storedEncryptedToken, encryptionKey)) {
        cerr << "integrity violation" << endl;
        return 255;
    }

    // Open the log file for reading
    ifstream log(logFile);
    if (!log.is_open()) {
        cerr << "integrity violation" << endl;
        return 255;
    }

    // Variables to store gallery state
    set<string> employeesInGallery, guestsInGallery;
    map<int, set<string>> roomOccupancy;
    map<string, vector<int>> personRoomHistory;

    string EncryptedLogEntry;
    while (getline(log, EncryptedLogEntry)) {
        string logEntry = decryptData(EncryptedLogEntry, encryptionKey);

        // Parse the log entry to determine event type and update the state
        istringstream entryStream(logEntry);
        string timestamp, type, name, action, roomStr;
        int roomID = -1;

        // Parse log entry assuming format: timestamp, type, name, action[, Room:roomID]
        getline(entryStream, timestamp, ',');
        getline(entryStream, type, ',');
        getline(entryStream, name, ',');
        getline(entryStream, action, ',');

        // Check if a room is specified in the format "Room:roomID"
        if (getline(entryStream, roomStr)) {
            size_t pos = roomStr.find("Room:");
            if (pos != string::npos) {
                try {
                    roomID = stoi(roomStr.substr(pos + 5));  // Parse room number after "Room:"
                }
                catch (const invalid_argument& e) {
                    cerr << "Invalid log entry" << endl;
                    continue;
                }
                catch (const out_of_range& e) {
                    cerr << "Invalid log entry" << endl;
                    continue;
                }
            }
        }

        // Process entry based on action and type
        if (action == "Arrival") {
            if (roomID == -1) {
                // Arrival to gallery
                if (type == "Employee") employeesInGallery.insert(name);
                else if (type == "Guest") guestsInGallery.insert(name);
            }
            else {
                // Arrival to room
                roomOccupancy[roomID].insert(name);
                personRoomHistory[name].push_back(roomID);
            }
        }
        else if (action == "Leave") {
            if (roomID == -1) {
                // Leave the gallery
                if (type == "Employee") employeesInGallery.erase(name);
                else if (type == "Guest") guestsInGallery.erase(name);
            }
            else {
                // Leave a specific room
                roomOccupancy[roomID].erase(name);
            }
        }
    }
    log.close();

    // Process the query type
    if (queryType == "State") {
        // Output current state of employees and guests in gallery
        for (const auto& emp : employeesInGallery) cout << emp << ",";
        cout << "\n";
        for (const auto& guest : guestsInGallery) cout << guest << ",";

        // Room-by-room listing
        for (const auto& room : roomOccupancy) {
            int roomID = room.first;
            const set<string>& occupants = room.second;
            cout << "\n" << roomID << ": ";
            for (const auto& person : occupants) cout << person << ",";
        }

        cout << endl;
    }
    else if (queryType == "RoomList") {
        if (persons.size() != 1) { cerr << "invalid" << endl; return 255; }
        // List rooms entered by the specified person in chronological order
        auto& roomHistory = personRoomHistory[persons[0]];
        for (size_t i = 0; i < roomHistory.size(); ++i) {
            if (i > 0) cout << ",";
            cout << roomHistory[i];
        }
        cout << endl;
    }

    return 0;
}