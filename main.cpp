/**
 * @file main.cpp
 * @author EMNEM
 * @brief Main file to start checkers robot logic on Raspberry Pi
 * @version 0.1
 * @date 2022-10-02
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <thread>
#include "PieceRecognition.h"
#include "FSM.h"
#include "Comms.h"
#include "ImageState.h"

int jimmy_depth = 10;
bool enable_jr = true;

void fsmLoop(FSM& fsm, volatile bool& calculating) {
    fsm.runThread();
    calculating = false;
}

int runCheckers(int argc, char** argv) {
    volatile bool calculating = false;
    if (argc > 1) {
        jimmy_depth = std::stoi(argv[1]);
    }
    if (argc > 2) {
        enable_jr = (bool) std::stoi(argv[2]);
    }
    FSM fsm;
    Comms uart;
    while(!uart.isConnected()) {
        uart.openConnection();
    }
    const char[11] blankPing = {0xFF, 0xFF, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    uart.sendData(blankPing, 11);
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    std::thread tCalc;
    // main loop
    while(true) {
        // UART
        char flags;
        bool newData;
        std::thread tCalc;
        if(uart.isConnected()) {
            newData = uart.checkData(flags);
            // Run FSM if new data received
            if(newData && fsm.dataSent) {
                if(!calculating) {
                    calculating = true;
                    fsm.currentFlags = flags;
                    //tCalc = std::thread(fsmLoop, std::ref(fsm), std::ref(calculating));
                    //tCalc.detach();
                    fsmLoop(fsm, calculating);
                }
            }
            else {
                if(!calculating && !fsm.dataSent) {
                    if(FLAG_SEND_MOVE & fsm.currentOutput[2])
                        std::cout << "Sent Move\n";
                    if(FLAG_SEND_MAJOR_FAULT & fsm.currentOutput[2]) {
                        std::cout << "Major Fault\n";
                    }
                    if(FLAG_SEND_WAIT_HOME & fsm.currentOutput[2]) {
                        std::cout << "Sent Wait Home\n";
                    }
                    if(FLAG_SEND_CAN_CAPTURE & fsm.currentOutput[2]) {
                        std::cout << "Sent Can Capture Flag\n";
                    }
                    uart.sendData(fsm.currentOutput, fsm.outputLength);
                    fsm.lastDataSent = std::chrono::system_clock::now();
                    fsm.dataSent = true;
                }
                else {
                    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
                    std::chrono::duration<double> diff = now - fsm.lastDataSent;
                    if(diff.count() > 0.3d) {
                        fsm.setOutput(FLAG_SEND_PING);
                    }
                }
            }
        }
        else {
            uart.openConnection();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
}

int main(int argc, char** argv) {
    //return testBoardAligner(argc, argv);
    //return testClusterizing(argc, argv);
    //return testBoardString(argc, argv);
    //return demoImageRecognition(argc, argv);
    return runCheckers(argc, argv);
}
