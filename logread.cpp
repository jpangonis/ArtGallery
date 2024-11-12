#include "log_utils.h" 
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <sstream>

//decrypt log entry when reading

int main(int argc, char* argv[]) {
    std::string token, logFile, queryType, personType, personName;
    std::vector<std::string> persons;
    bool multiplePersons = false;

    // Parse command-line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-K") token = argv[++i];
        else if (arg == "-S") queryType = "State";
        else if (arg == "-R") queryType = "RoomList";
        else if (arg == "-T") queryType = "TotalTime";
        else if (arg == "-I") { queryType = "SharedRooms"; multiplePersons = true; }
        else if (arg == "-E") { personType = "Employee"; persons.push_back(argv[++i]); }
        else if (arg == "-G") { personType = "Guest"; persons.push_back(argv[++i]); }
        else logFile = argv[i];
    }

    // Check if only one query type is specified
    if (queryType.empty() || logFile.empty() || token.empty()) {
        std::cerr << "invalid" << std::endl;
        return 255;
    }

    // Load and encrypt the stored token for verification
    const std::string encryptionKey = "thisisasecretkey";
    std::string storedEncryptedToken = encryptToken("secret", encryptionKey);

    // Verify provided token
    if (!verifyToken(token, storedEncryptedToken, encryptionKey)) {
        std::cerr << "integrity violation" << std::endl;
        return 255;
    }

    // Open the log file for reading
    std::ifstream log(logFile);
    if (!log.is_open()) {
        std::cerr << "integrity violation" << std::endl;
        return 255;
    }

    // Variables to store gallery state
    std::set<std::string> employeesInGallery, guestsInGallery;
    std::map<int, std::set<std::string>> roomOccupancy;
    std::map<std::string, std::vector<int>> personRoomHistory;

    std::string logEntry;
    while (std::getline(log, logEntry)) {
        // Parse the log entry to determine event type and update the state
        std::istringstream entryStream(logEntry);
        std::string timestamp, type, name, action, roomStr;
        int roomID = -1;

        // Parse log entry assuming format: timestamp, type, name, action[, Room:roomID]
        std::getline(entryStream, timestamp, ',');
        std::getline(entryStream, type, ',');
        std::getline(entryStream, name, ',');
        std::getline(entryStream, action, ',');

        // Check if a room is specified in the format "Room:roomID"
        if (std::getline(entryStream, roomStr)) {
            size_t pos = roomStr.find("Room:");
            if (pos != std::string::npos) {
                try {
                    roomID = std::stoi(roomStr.substr(pos + 5));  // Parse room number after "Room:"
                }
                catch (const std::invalid_argument& e) {
                    std::cerr << "Invalid log entry" << std::endl;
                    continue; 
                }
                catch (const std::out_of_range& e) {
                    std::cerr << "Invalid log entry" << std::endl;
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
        for (const auto& emp : employeesInGallery) std::cout << emp << ",";
        std::cout << "\n";
        for (const auto& guest : guestsInGallery) std::cout << guest << ",";

        // Room-by-room listing
        for (const auto& room : roomOccupancy) {
            int roomID = room.first;
            const std::set<std::string>& occupants = room.second;
            std::cout << "\n" << roomID << ": ";
            for (const auto& person : occupants) std::cout << person << ",";
        }

        std::cout << std::endl;
    }
    else if (queryType == "RoomList") {
        if (persons.size() != 1) { std::cerr << "invalid" << std::endl; return 255; }
        // List rooms entered by the specified person in chronological order
        auto& roomHistory = personRoomHistory[persons[0]];
        for (size_t i = 0; i < roomHistory.size(); ++i) {
            if (i > 0) std::cout << ",";
            std::cout << roomHistory[i];
        }
        std::cout << std::endl;
    }
    else if (queryType == "TotalTime") {
        if (persons.size() != 1) { std::cerr << "unimplemented" << std::endl; return 0; }
        // Calculate total time spent by the specified person
        std::cout << "unimplemented" << std::endl;
    }
    else if (queryType == "SharedRooms") {
        if (!multiplePersons || persons.empty()) { std::cerr << "invalid" << std::endl; return 255; }
        // Find rooms occupied simultaneously by all specified persons
        std::cout << "unimplemented" << std::endl;
    }

    return 0;
}
