/**
 * @file FSM.cpp
 * @author EMNEM
 * @brief Keeps track of the state of the checkers game
 * @version 0.1
 * @date 2022-11-03
 */

#include <iostream>
#include <list>
#include <chrono>
#include <opencv2/opencv.hpp>
#include "FSM.h"
#include "ImageState.h"
#include "PieceRecognition.h"
#include "bot.h"

FSM::FSM() {
    state = WAIT_FOR_PLAYER;
    tempNextState = WAIT_FOR_PLAYER;
    currentFlags = 0;
    dataSent = true;
    jimmy.evalFunc = &Bot::evalIV;
    jimmy.max_depth = jimmy_depth;
    lastDataSent = std::chrono::system_clock::now();
    aligned = false;
    gameStarted = false;
}

void FSM::nextState() {
    char sendFlags = 0;
    //std::cout << (int) currentFlags << std::endl;
    switch(state) {
        case WAIT_FOR_PLAYER:
            if(currentFlags & FLAG_RECV_ALIGN) {
                std::cout << "Aligning\n";
                cv::Mat img;
                bool imgSuccess = takePicture(img);
                if(imgSuccess) {
                    bool boardSuccess = boardState.alignCamera(img);
                    if(!boardSuccess) {
                        sendFlags |= FLAG_SEND_MAJOR_FAULT | FLAG_SEND_WAIT_HOME;
                    }
                    else {
                        std::cout << "Image Aligned SUccessfully\n";
                        aligned = true;
                    }
                }
                else {
                    sendFlags |= FLAG_SEND_MAJOR_FAULT | FLAG_SEND_WAIT_HOME;
                }
            }
            else if(currentFlags & FLAG_RECV_START) {
                std::cout << "Starting new game\n";
                tempNextState = SEND_MOVES;
                cv::Mat img;
                bool imgSuccess = takePicture(img);
                if(imgSuccess && aligned) {
                    bool boardSuccess = boardState.generateBoardstate(img, false);
                    jimmy.init_board();
                    if(boardSuccess) {
                        boardState.createMoveList(moveList);
                        std::cout << "Move list created\n";
                        if(boardState.majorFault) {
                            sendFlags |= FLAG_SEND_MAJOR_FAULT | FLAG_SEND_WAIT_HOME;
                            moveList.clear();
                            tempNextState = WAIT_FOR_PLAYER;
                        }
                        else if(moveList.size() == 0) {
                            tempNextState = WAIT_FOR_PLAYER;
                            gameStarted = true;
                            std::memcpy(boardState.lastValidBoardState, boardState.STARTING_BOARD, sizeof(boardState.STARTING_BOARD));
                        }
                        else {
                            sendFlags |= FLAG_SEND_MOVE;
                            gameStarted = true;
                            std::memcpy(boardState.lastValidBoardState, boardState.STARTING_BOARD, sizeof(boardState.STARTING_BOARD));
                        }
                    }
                    else {
                        sendFlags |= FLAG_SEND_MAJOR_FAULT | FLAG_SEND_WAIT_HOME;
                        moveList.clear();
                        tempNextState = WAIT_FOR_PLAYER;
                    }
                }
                else {
                    sendFlags |= FLAG_SEND_MAJOR_FAULT | FLAG_SEND_WAIT_HOME;
                    moveList.clear();
                    tempNextState = WAIT_FOR_PLAYER;
                }
            }
            else if(currentFlags & FLAG_RECV_PLAYER_MOVE) {
                // Bot makes next move
                tempNextState = SEND_MOVES;
                cv::Mat img;
                bool imgSuccess = takePicture(img);
                if(imgSuccess && aligned && gameStarted) {
                    bool boardSuccess = boardState.generateBoardstate(img);
                    if(boardState.majorFault) {
                        // Big uh oh do nothing
                        sendFlags |= FLAG_SEND_MAJOR_FAULT | FLAG_SEND_WAIT_HOME;
                        tempNextState = WAIT_FOR_PLAYER;
                    }
                    else if(!boardSuccess) {
                        // Go back to previous good board
                        boardState.createMoveList(moveList, boardState.lastValidBoardState);
                        if(boardState.majorFault) {
                            sendFlags |= FLAG_SEND_MAJOR_FAULT | FLAG_SEND_WAIT_HOME;
                            moveList.clear();
                            tempNextState = WAIT_FOR_PLAYER;
                            std::cout << "Error generating moves\n";
                        }
                        else if(moveList.size() == 0) {
                            sendFlags |= FLAG_SEND_WAIT_HOME;
                            tempNextState = WAIT_FOR_PLAYER;
                        }
                        else {
                            std::cout << "Incorrect Move or invalid piece detected\n";
                            sendFlags |= FLAG_SEND_MOVE;
                            sendFlags |= FLAG_SEND_ILLEGAL_MOVE;
                        }
                    }
                    else {
                        // Generate computer move
                        std::cout << "Generating Move\n";
                        std::cout << "Number of boards analyzed: "<< jimmy.gen_move(boardState.boardState) << std::endl;
                        printBoardState(jimmy.board);
                        std::memcpy(boardState.lastValidBoardState, jimmy.board, sizeof(jimmy.board));
                        // std::cout << jimmy.eval() << '\n' << std::endl;
                        boardState.createMoveList(moveList, jimmy.board);
                        std::cout << "Moves to make: " << (int)moveList.size() << std::endl;
                        if(boardState.majorFault) {
                            sendFlags |= FLAG_SEND_MAJOR_FAULT | FLAG_SEND_WAIT_HOME;
                            moveList.clear();
                            tempNextState = WAIT_FOR_PLAYER;
                        }
                        else if(moveList.size() == 0) {
                            sendFlags |= FLAG_SEND_WAIT_HOME;
                            tempNextState = WAIT_FOR_PLAYER;
                        }
                        else {
                            sendFlags |= FLAG_SEND_MOVE;
                        }
                    }
                }
                else {
                    sendFlags |= FLAG_SEND_MAJOR_FAULT;
                    sendFlags |= FLAG_SEND_WAIT_HOME;
                    tempNextState = WAIT_FOR_PLAYER;
                }
            }
            else {
                //std::cout << "No flags received\n";
            }
            break;
        case SEND_MOVES:
            if(currentFlags & FLAG_RECV_DONE) {
                std::cout << "movelist.size(): " << (int)moveList.size() << "\n";
                if(moveList.size() == 0) {
                    sendFlags |= FLAG_SEND_WAIT_HOME;
                    tempNextState = WAIT_FOR_PLAYER;
                }
                else {
                    // Send the next move
                    sendFlags |= FLAG_SEND_MOVE;
                }
            }
            break;
    }
    outputFlags = sendFlags;
}

void FSM::outputState() {
    switch(tempNextState) {
        case WAIT_FOR_PLAYER:
            if(jimmy.can_cap() && state == SEND_MOVES) {
                outputFlags |= FLAG_SEND_CAN_CAPTURE;
            }
            if(outputFlags) {
                setOutput(outputFlags);
            }
            break;
        case SEND_MOVES:
            if(moveList.size() > 0 && (outputFlags & FLAG_SEND_MOVE)) {
                setOutput(outputFlags, moveList.front());
                moveList.pop_front();
            }
            break;
    }
    state = tempNextState;
    return;
}

void FSM::runThread() {
    // update state
    nextState();
    outputState();
}

void FSM::setOutput(char flags) {
    ImageMove noMove;
    noMove.startX = 0;
    noMove.startY = 0;
    noMove.endX = 0;
    noMove.endY = 0;
    setOutput(flags, noMove);
}

void FSM::setOutput(char flags, ImageMove& move) {
    int moveStartX = 0;
    int moveStartY = 0;
    int moveEndX = 0;
    int moveEndY = 0;
    boardState.imageToRealSpace(moveStartX, moveStartY, move.startX, move.startY);
    boardState.imageToRealSpace(moveEndX, moveEndY, move.endX, move.endY);
    currentOutput[0] = (char)0xFF;
    currentOutput[1] = (char)0xFF;
    currentOutput[2] = flags;
    currentOutput[3] = (char)(moveStartX);
    currentOutput[4] = (char)(moveStartX >> 8);
    currentOutput[5] = (char)(moveStartY);
    currentOutput[6] = (char)(moveStartY >> 8);
    currentOutput[7] = (char)(moveEndX);
    currentOutput[8] = (char)(moveEndX >> 8);
    currentOutput[9] = (char)(moveEndY);
    currentOutput[10] = (char)(moveEndY >> 8);
    dataSent = false;
    outputLength = 11;
}
