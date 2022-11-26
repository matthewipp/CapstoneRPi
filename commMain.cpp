#include "Comms.h"
#include <wiringPi.h>
#include <iostream>

int main() {
    std::cout << "WiringPi Setup: " << wiringPiSetup() << std::endl;
    Comms c;
    unsigned int time;


    std::cout << "Established connection: " << c.openConnection() << std::endl;
    std::cout << "Comm file descriptor: " << c.getFD() << std::endl;
    std::cout << "# of chars in buffer: " << serialDataAvail(c.getFD()) << std::endl;
    std::cout << "Sending ping... " << std::endl;
    // std::string message = "Hello";
    // c.sendData(message.c_str(), 5);
    char message[] = {0xFF, 0xFF, 0x02, 0x01, 0x01, 0x01, 0x01, 0x00, 0x01, 0x01, 0x01};
    // char message[] = {0xFF, 0xFF, 0x04, 0x1A, 0x04, 0x1A, 0x04, 0xD6, 0x06, 0xD6, 0x06};
    // char message[] = {0xFF, 0xFF, 0x01, 0x1A, 0x04, 0x1A, 0x04, 0xD6, 0x06, 0xD6, 0x06};
    c.sendData(message, 11);
    time = millis();
    while (millis() < time + 1000) {

    }
    std::cout << "# of chars in buffer: " << serialDataAvail(c.getFD()) << std::endl;
    std::cout << "Read from buffer character at a time:" << std::endl;
    char data;
    while (c.checkData(data)) {
        std::cout << (int) data << ": " << data << std::endl;
    }
    std::cout << "Done" << std::endl;

    return 0;
}

