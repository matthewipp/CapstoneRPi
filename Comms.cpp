/**
 * @file Comms.cpp
 * @author EMNEM
 * @brief Windows stuff for comms
 * @version 0.1
 * @date 2022-11-16
 */

#include <iostream>
#include "Comms.h"

Comms::Comms() {
    connected = false;
    fd = -1;
}

int Comms::getFD() {
    return fd;
}

bool Comms::isConnected() {
    return connected;
}

bool Comms::openConnection() {
    fd = serialOpen(SERIAL_PORT, 9600);
    if (fd == -1) {
        // Error handling?
        connected = false;
    } else {
        connected = true;
    }
    return connected;
}

bool Comms::closeConnection() {
    serialClose(fd);
    connected = false;
    return true;
}

bool Comms::checkData(char& flags) {
#   if TEST_MODE

    int data;
    std::cout << "Enter simulated flags byte from the MSP430: ";
    std::cin >> data;
    flags = (char)data;
    return (bool)data;

#   else

    if (serialDataAvail(fd) > 0) {
        flags = (char) serialGetchar(fd);
        return true;
    }
    return false;

#   endif
}

void Comms::sendData(const char* message, int len) {
#   if TEST_MODE

    std::cout << "Sending data\n";
    for(int i = 0; i < len; i++) {
        std::cout << (int)(message[i]) << " | ";
    }
    std::cout << std::endl;
    return;

#   else 

    // NOTE: Requires null-terminated message
    serialPuts(fd, message);

#   endif
}

void Comms::flush() {
    serialFlush(fd);
}

// int main() {

//     return 0;
// }
