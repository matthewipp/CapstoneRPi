/**
 * @file FSM.cpp
 * @author EMNEM
 * @brief Keeps track of the state of the checkers game
 * @version 0.1
 * @date 2022-11-03
 */

#include <iostream>
#include <list>
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
}

void FSM::nextState() {
    char sendFlags = 0;
    switch(state) {
        case WAIT_FOR_PLAYER:
            if(currentFlags & FLAG_RECV_NONE) {
                std::cout << "No flags received\n";
            }
            else if(currentFlags & FLAG_RECV_ALIGN) {
                std::cout << "Aligning\n";
                cv::Mat img;
                bool imgSuccess = takePicture(img);
                if(imgSuccess) {
                    bool boardSuccess = boardState.alignCamera(img);
                    if(!boardSuccess) {
                        sendFlags |= FLAG_SEND_MAJOR_FAULT;
                    }
                    else {
                        std::cout << "Image Aligned SUccessfully\n";
                    }
                }
                else {
                    sendFlags |= FLAG_SEND_MAJOR_FAULT;
                }
            }
            else if(currentFlags & FLAG_RECV_START) {
                std::cout << "Starting new game\n";
                tempNextState = SEND_MOVES;
                cv::Mat img;
                bool imgSuccess = takePicture(img);
                if(imgSuccess) {
                    bool boardSuccess = boardState.generateBoardstate(img, false);
                    std::cout << "Board Read Successfully\n";
                    /*if(boardSuccess) {
                        boardState.createMoveList(moveList);
                        std::cout << "Move list created\n";
                        if(boardState.majorFault) {
                            sendFlags |= FLAG_SEND_MAJOR_FAULT;
                            tempNextState = WAIT_FOR_PLAYER;
                        }
                        else if(moveList.size() == 0) {
                            tempNextState = WAIT_FOR_PLAYER;
                        }
                        else {
                            sendFlags |= FLAG_SEND_MOVE;
                        }
                    }
                    else {
                        sendFlags |= FLAG_SEND_MAJOR_FAULT;
                        tempNextState = WAIT_FOR_PLAYER;
                    }*/
                }
                else {
                    sendFlags |= FLAG_SEND_MAJOR_FAULT;
                    tempNextState = WAIT_FOR_PLAYER;
                }
            }
            else if(currentFlags & FLAG_RECV_PLAYER_MOVE) {
                // Bot makes next move
                tempNextState = SEND_MOVES;
                cv::Mat img;
                bool imgSuccess = takePicture(img);
                if(imgSuccess) {
                    bool boardSuccess = boardState.generateBoardstate(img);
                    if(boardState.majorFault) {
                        // Big uh oh do nothing
                        sendFlags |= FLAG_SEND_MAJOR_FAULT;
                    }
                    else if(!boardSuccess) {
                        // Go back to previous good board
                        boardState.createMoveList(moveList);
                        if(boardState.majorFault) {
                            sendFlags |= FLAG_SEND_MAJOR_FAULT;
                            tempNextState = WAIT_FOR_PLAYER;
                        }
                        else if(moveList.size() == 0) {
                            tempNextState = WAIT_FOR_PLAYER;
                        }
                        else {
                            sendFlags |= FLAG_SEND_MOVE;
                        }
                    }
                    else {
                        // Generate computer move
                        jimmy.gen_move(boardState.boardState);
                        /*boardState.createMoveList(moveList, jimmy.board);
                        if(boardState.majorFault) {
                            sendFlags |= FLAG_SEND_MAJOR_FAULT;
                            tempNextState = WAIT_FOR_PLAYER;
                        }
                        else if(moveList.size() == 0) {
                            tempNextState = WAIT_FOR_PLAYER;
                        }
                        else {
                            sendFlags |= FLAG_SEND_MOVE;
                        }*/
                    }
                }
                else {
                    sendFlags |= FLAG_SEND_MAJOR_FAULT;
                }
            }
            break;
        case SEND_MOVES:
            if(currentFlags & FLAG_RECV_DONE) {
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
            if(outputFlags) {
                setOutput(outputFlags);
            }
            break;
        case SEND_MOVES:
            setOutput(outputFlags, moveList.front());
            moveList.pop_front();
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
    setOutput(flags, noMove);
}

void FSM::setOutput(char flags, ImageMove& move) {
    currentOutput[0] = (char)0xFF;
    currentOutput[1] = (char)0xFF;
    currentOutput[2] = flags;
    currentOutput[3] = (char)(move.startX >> 8);
    currentOutput[4] = (char)(move.startX);
    currentOutput[5] = (char)(move.startY >> 8);
    currentOutput[6] = (char)(move.startY);
    currentOutput[7] = (char)(move.endX >> 8);
    currentOutput[8] = (char)(move.endX);
    currentOutput[9] = (char)(move.endY >> 8);
    currentOutput[10] = (char)(move.endY);
    dataSent = false;
    outputLength = 11;
}
