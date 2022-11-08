/**
 * @file FSM.h
 * @author EMNEM
 * @brief Defines the FSM for communicating with the MSP430
 * @version 0.1
 * @date 2022-11-03
 */

#ifndef FSM_H
#define FSM_H

enum FSMState {WAIT_FOR_PLAYER, START_GAME, MAKE_MOVE, WAIT_FOR_COMPLETION};

#define FLAG_NOTHING 0x00
#define FLAG_HOME 0x01
#define FLAG_ALIGN 0x02
#define FLAG_START_GAME 0x04
#define FLAG_MAKE_MOVE 0x08
#define FLAG_DONE_TASK 0x10
#define FLAG_FORCE_HOME 0x20
#define FLAG_ERROR 0x40

#include "ImageState.h"

class FSM {
    public:
        FSM(bool playerFirst);
        // Runs next state and returns data to be communicated with 
        char* runThread(char* flags, int& outputLen);
        bool playerMove;
        bool inGame;
        FSMState state;
        FSMState tempNextState;
        int currentFlags;
        ImageState boardState;
    private:
        // Calculates the next state
        void nextState();
        // Switches state, should only be run when running thread is about to sleep
        char* outputState(int& length);
        // convert move struct to char array
        std::string convertMoveToComms(ImageMove& move);
};

#endif
