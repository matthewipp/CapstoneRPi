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

int testClusterizing(int argc, char** argv) {
    // Check arguments
    if(argc != 2) {
        std::cout << "Must use exactly 1 argument, the file path\n";
        return 0;
    }
    // Read image
    std::string filename = argv[1];
    cv::Mat img = cv::imread(filename);
    if(img.empty()) {
        std::cout << "Could not read file: " << filename << "\n"; 
    }
    std::cout << "Successfully Read file: " << filename << "\n";
    // Get points lists
    int test = CLUSTER_MIN_POINTS;
    std::vector<std::vector<Point>> points;
    getPointsInImage(img, points);
    std::vector<Point> bluePoints = points.back();
    points.pop_back();
    std::vector<Point> redPoints = points.back();
    points.pop_back();
    // Write points to file
    /*
    std::ofstream fStream;
    fStream.open("Coutput.txt");
    for(Point p : bluePoints) {
        fStream << "(" << p.x << ", " << p.y << ") " << p.type << "\n";
    }
    */
    // Print points
    std::cout << "There are " << bluePoints.size() << " blue points and ";
    std::cout << redPoints.size() << " red points\n";
    // Clusterize
    std::vector<Cluster> blueClusters;
    clusterize(bluePoints, true, blueClusters);
    std::vector<Cluster> redClusters;
    clusterize(redPoints, false, redClusters);
    // Print clusters
    std::cout << "There are " << blueClusters.size() << " blue clusters and ";
    std::cout << redClusters.size() << " red clusters\n";

    return 0;
}

int testBoardAligner(int argc, char** argv) {
    // Check arguments
    if(argc != 2) {
        std::cout << "Must use exactly 1 argument, the file path\n";
        return 0;
    }
    // Read image
    std::string filename = argv[1];
    cv::Mat img = cv::imread(filename);
    if(img.empty()) {
        std::cout << "Could not read file: " << filename << "\n"; 
        return 0;
    }
    // Align board
    ImageState boardState;
    boardState.alignCamera(img);
    return 0;
}

int testBoardString(int argc, char** argv) {
    // Check arguments
    if(argc != 3) {
        std::cout << "Must use exactly 2 arguments, the align file path and the image file path\n";
        return 0;
    }
    // Read images
    std::string alignFilename = argv[1];
    std::string stateFilename = argv[2];
    cv::Mat alignImg = cv::imread(alignFilename);
    if(alignImg.empty()) {
        std::cout << "Could not read file: " << alignFilename << "\n"; 
        return 0;
    }
    cv::Mat stateImg = cv::imread(stateFilename);
    if(stateImg.empty()) {
        std::cout << "Could not read file: " << stateFilename << "\n"; 
        return 0;
    }
    // Align board
    ImageState boardState;
    bool aligned = boardState.alignCamera(alignImg);
    if(!aligned) {
        std::cout << "Could not align board\n";
        return 0;
    }
    std::cout << "Aligned Successfully!\n";
    // Get board state
    bool majorFault = false;
    bool yay = boardState.generateBoardstate(stateImg, majorFault);
    if(yay) {
        std::cout << "Successfully imaged board: \n";
        std::cout << boardState.boardState << "\n";
    }
    else {
        std::cout << "Detected invalid board\n";
    }
    std::cout << "Board left and right side: " << boardState.edgeX[0] << ", " << boardState.edgeX[1] << "\n";
    std::cout << "Board top and bottom side: " << boardState.edgeY[0] << ", " << boardState.edgeY[1] << "\n";
    std::cout << "Average Square Size (widthxheight): " << boardState.avgSquareWidth << " x ";
    std::cout << boardState.avgSquareHeight << "\n";
    std::cout << "Red Pieces: " << boardState.redPiecesOnBoard.size() << "\n";
    std::cout << "Blue Pieces: " << boardState.bluePiecesOnBoard.size() << "\n";
    return 0;
}

int demoImageRecognition(int argc, char** argv) {
    bool running = true;
    // Connect to camera
    cv::Mat img;
    std::string command;
    ImageState boardState;
    int totalAttempts = 0;
    int successfulAttempts = 0;
    // Run loop
    while(running) {
        std::cout << "Waiting for command: align, image, reset, show, exit\n";
        std::cin >> command;
        if(command == "align") {
            // Take image and align boardstate
            bool camSuccess = takePicture(img);
            if(camSuccess) {
                bool alignSuccess = boardState.alignCamera(img);
                if(alignSuccess) {
                    std::cout << "Successfully aligned board";
                    std::cout << "Board left and right side: " << boardState.edgeX[0] << ", " << boardState.edgeX[1] << "\n";
                    std::cout << "Board top and bottom side: " << boardState.edgeY[0] << ", " << boardState.edgeY[1] << "\n";
                    std::cout << "Average Square Size (widthxheight): " << boardState.avgSquareWidth << " x ";
                    std::cout << boardState.avgSquareHeight << "\n";
                }
                else {
                    std::cout << "Could not align board\n";
                }
            }
            else {
                std::cout << "Error reading from camera\n";
            }
        }
        else if(command == "image") {
            // Take image and anlyze the board state
            bool camSuccess = takePicture(img);
            if(camSuccess) {
                bool majorFault = false;
                bool stateSuccess = boardState.generateBoardstate(img, majorFault);
                if(stateSuccess) {
                    std::cout << "BoardState Successfully generated\n";
                }
                else {
                    std::cout << "Error Generating Board State\n";
                }
                printBoardState(boardState.boardState);
                std::cout << "Was this the expected result (Y/n)?";
                std::string response;
                bool valid = false;
                totalAttempts += 1;
                while(!valid) {
                    std::cin >> response;
                    if(response == "y" || response == "y") {
                        successfulAttempts += 1;
                        valid = true;
                    }
                    if(response == "n" || response == "N") {
                        valid = true;
                    }
                }
            }
            else {
                std::cout << "Error reading from camera\n";
            }
        }
        else if(command == "reset") {
            totalAttempts = 0;
            successfulAttempts = 0;
            boardState = ImageState();
            std::cout << "totalAttempts and successful attempts reset to 0\n";
            std::cout << "boardState reinitialized, must realign\n";
        }
        else if(command == "show") {
            std::cout << successfulAttempts << " / " << totalAttempts << "\n";
            if(totalAttempts != 0) {
                std::cout << (float)successfulAttempts / (float)totalAttempts * 100 << "%\n";
            }
        }
        else if(command == "exit") {
            running = false;
        }
        else {
            std::cout << "Command not recognized\n";
        }
    }
    return 0;
}

void fsmLoop(FSM& fsm, volatile bool& calculating) {
    fsm.runThread();
    calculating = false;
}

int runCheckers(int argc, char** argv) {
    volatile bool calculating = false;
    FSM fsm;
    Comms uart;
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
            if(newData) {
                if(!calculating) {
                    calculating = true;
                    fsm.currentFlags = flags;
                    tCalc = std::thread(fsmLoop, std::ref(fsm), std::ref(calculating));
                    tCalc.detach();
                }
            }
            else {
                if(!calculating && !fsm.dataSent) {
                    uart.sendData(fsm.currentOutput, fsm.outputLength);
                    fsm.dataSent = true;
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
