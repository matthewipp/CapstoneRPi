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
    connected = true;
    return true;
}

bool Comms::closeConnection() {
    connected = false;
    return true;
}

bool Comms::checkData(char& flags) {
    int data;
    std::cout << "Enter simulated flags byte from the MSP430: ";
    std::cin >> data;
    flags = (char)data;
    return true;
}

void Comms::sendData(char* message, int len) {
    std::cout << "Sending data\n";
    for(int i = 0; i < len; i++) {
        std::cout << (int)(message[i]) << " | ";
    }
    std::cout << std::endl;
    return;
}

void Comms::flush() {

}
