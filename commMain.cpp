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
    std::cout << "Sending message 'Hello'... " << std::endl;
    std::string message = "Hello";
    c.sendData(message.c_str(), 5);
    time = millis();
    while (millis() < time + 6) {

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

