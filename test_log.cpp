#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <cassert>

// Utility function to check the content of a file
std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return "";
    }
    std::string content((std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());
    file.close();
    return content;
}

// Utility function to run a command and capture the result
int runCommand(const std::string& command) {
    return system(command.c_str());
}

// Test cases for logappend and logread
void testLogAppend() {
    std::cout << "Running logappend tests..." << std::endl;

    // Cleanup before running tests
    runCommand("del log1"); // Use "del" for Windows

    // Test 1: Valid logappend command
    assert(runCommand("logappend.exe -T 1 -K secret -A -E Fred log1") == 0);

    // Test 2: Invalid timestamp (out of order)
    assert(runCommand("logappend.exe -T 3 -K secret -A -E Fred -R 1 log1") == 0);

    // Test 3: Invalid name (contains numbers)
    assert(runCommand("logappend.exe -T 6 -K secret -A -E Fred123 -R 1 log1") != 0);

    // Test 4: Invalid room ID (non-numeric)
    assert(runCommand("logappend.exe -T 7 -K secret -A -E Fred -R RoomA log1") != 0);

    // Test 5: Adding a guest
    assert(runCommand("logappend.exe -T 8 -K secret -A -G Alice -R 2 log1") != 0);

    //Test 6: wrong token
    assert(runCommand("logappend.exe -T 9 -K wrongsecret -A -G Alice -R 2 log1") != 0);

    //Test 7: leaving gallery before leaving room
    assert(runCommand("logappend.exe -T 10 -K secret -L -G Alice log1") != 0);

    //Test 8: leaving room correctly
    assert(runCommand("logappend.exe -T 11 -K secret -L -E Fred -R 1 log1") == 0);

    //Test 9: incorrect order
    assert(runCommand("logappend.exe -T 2 -K secret -A -G Fred -R 2 log1") != 0);

    //Test 10: leaving room before entering
    assert(runCommand("logappend.exe -T 12 -K secret -L -E Fred -R 2 log1") != 0);

    std::cout << "logappend tests passed!" << std::endl;
}

void testLogRead() {
    std::cout << "Running logread tests..." << std::endl;

    // Cleanup before running tests
    runCommand("del log2"); // Use "del" for Windows

    // Prepare a log file using logappend
    runCommand("logappend.exe -T 1 -K secret -A -E Fred log2");
    runCommand("logappend.exe -T 2 -K secret -A -G Alice log2");
    runCommand("logappend.exe -T 3 -K secret -A -E Fred -R 1 log2");
    runCommand("logappend.exe -T 4 -K secret -A -G Alice -R 1 log2");

    // Test 1: Query state of the gallery
    assert(runCommand("logread.exe -K secret -S log2") == 0);

    // Test 2: Query room list for an employee
    assert(runCommand("logread.exe -K secret -R -E Fred log2") == 0);

    // Test 3: Invalid token
    assert(runCommand("logread.exe -K wrongsecret -S log2") != 0);

    // Test 4: Query with an empty log file
    runCommand("del log3"); // Use "del" for Windows
    assert(runCommand("logread.exe -K secret -S log3") != 0);

    // Test 5: Unauthorized access attempt
    // Attempt to read without providing a key
    assert(runCommand("logread.exe -S log2") != 0);

    // Test 6: Attempt to access data for an unspecified employee
    assert(runCommand("logread.exe -K secret -R -E Nonexistent log2") == 0);

    // Test 7: Attempt to access data for a room without specifying the room parameter
    assert(runCommand("logread.exe -K secret -R log2") != 0);


    std::cout << "logread tests passed!" << std::endl;
}

int main() {
    testLogAppend();
    testLogRead();

    std::cout << "All tests passed!" << std::endl;
    return 0;
}
