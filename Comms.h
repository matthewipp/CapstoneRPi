/**
 * @file Comms.h
 * @author EMNEM
 * @brief handles comms using serial UART
 * @version 0.1
 * @date 2022-11-03
 */

#ifndef COMMS_H
#define COMMS_H

#define TEST_MODE   0
// #define SERIAL_PORT "/tty/dev/0"
#define SERIAL_PORT "/dev/ttyS0"
//http://wiringpi.com/reference/serial-library/

#include <wiringSerial.h>

class Comms {
    public:
        Comms();
        bool isConnected();
        int getFD();
        bool openConnection();
        bool closeConnection();
        bool checkData(char& flags);
        void sendData(const char* message, int len);
        void flush();
    private:
        bool connected;
        int fd;
};

#endif
