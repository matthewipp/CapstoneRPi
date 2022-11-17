/**
 * @file FSM.h
 * @author EMNEM
 * @brief Defines the FSM for communicating with the MSP430
 * @version 0.1
 * @date 2022-11-03
 */

#ifndef FSM_H
#define FSM_H

#include "ImageState.h"

// WAIT_FOR_PLAYER waits for player to hit button
// SEND_MOVES
enum FSMState {WAIT_FOR_PLAYER, SEND_MOVES};

// Flags recieved from MSP
#define FLAG_RECV_NONE 0x01
#define FLAG_RECV_ALIGN 0x02
#define FLAG_RECV_START 0x04
#define FLAG_RECV_PLAYER_MOVE 0x08
#define FLAG_RECV_DONE 0x10
// Flags to send
#define FLAG_SEND_WAIT_HOME 0x01
#define FLAG_SEND_PING 0x02
#define FLAG_SEND_MOVE 0x04
#define FLAG_SEND_MAJOR_FAULT 0x08
#define FLAG_SEND_ILLEGAL_MOVE 0x10

#include "ImageState.h"
#include "bot.h"

class FSM {
    public:
        FSM();
        // Runs next state and returns data to be communicated with 
        void runThread();
        FSMState state;
        FSMState tempNextState;
        char currentFlags;
        ImageState boardState;
        char currentOutput[11];
        char outputFlags;
        int outputLength;
        std::list<ImageMove> moveList;
        bool dataSent;
        Bot jimmy;
    private:
        // Calculates the next state
        void nextState();
        // Switches state, should only be run when running thread is about to sleep
        void outputState();
        // sets the output array to the message
        void setOutput(char flags);
        // sets the output array to the message with a move
        void setOutput(char flags, ImageMove& move);
        // convert move struct to char array
        std::string convertMoveToComms(ImageMove& move);
};

#endif
