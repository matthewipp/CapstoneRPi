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

FSM::FSM() {
    state = WAIT_FOR_PLAYER;
    currentFlags = 0;
    dataSent = true;
}

void FSM::nextState() {
    state = tempNextState;
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
                    if(boardSuccess) {
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
                        sendFlags |= FLAG_SEND_MAJOR_FAULT;
                        tempNextState = WAIT_FOR_PLAYER;
                    }
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
                        char desiredBoardState[8][8];
                        boardState.createMoveList(moveList, desiredBoardState);
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
            setOutput(outputFlags);
            break;
        case SEND_MOVES:
            setOutput(outputFlags, moveList.front());
            moveList.pop_front();
            break;
    }
    return;
}

void FSM::runThread() {
    // update state
    nextState();
    outputState();
}
