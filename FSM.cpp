/**
 * @file FSM.cpp
 * @author EMNEM
 * @brief Keeps track of the state of the checkers game
 * @version 0.1
 * @date 2022-11-03
 */

#include "FSM.h"
#include "ImageState.h"

FSM::FSM(bool playerFirst) {
    playerMove = playerFirst;
    inGame = false;
    state = WAIT_FOR_PLAYER;
    currentFlags = 0;
}

void FSM::nextState() {
    currentFlags = 0;
    switch(state) {
        case WAIT_FOR_PLAYER:
            if(currentFlags & FLAG_HOME) {
                currentFlags |= FLAG_FORCE_HOME;
                tempNextState = WAIT_FOR_COMPLETION;
            }
            if(currentFlags & FLAG_ALIGN) {
                currentFlags |= FLAG_HOME | FLAG_ALIGN;
                tempNextState = WAIT_FOR_COMPLETION;
            }
            if(currentFlags & FLAG_START_GAME) {
                currentFlags |= FLAG_HOME | FLAG_ALIGN;
                tempNextState = START_GAME;
            }
            if(currentFlags & MAKE_MOVE) {
                currentFlags |= FLAG_HOME | FLAG_ALIGN;
                tempNextState = MAKE_MOVE;
            }
            if(currentFlags == 0) {
                // Should not get here
                currentFlags = FLAG_ERROR;
                tempNextState = WAIT_FOR_COMPLETION;
            }
            break;
        case START_GAME:
        case MAKE_MOVE:
            // Should not get here
            currentFlags = FLAG_ERROR;
            tempNextState = WAIT_FOR_PLAYER;
            break;
        case WAIT_FOR_COMPLETION:
            if(FLAG_DONE_TASK) {
                tempNextState = WAIT_FOR_PLAYER;
            }
            break;
    }
}

char* FSM::outputState(int& length) {
    char* output = nullptr;
    switch(tempNextState) {
        case WAIT_FOR_COMPLETION:
            // Only flags, no arguments
            output = new char[1];
            output[0] = (char)currentFlags;
            break;
        case START_GAME:
        case MAKE_MOVE:
            std::vector<ImageMove> moves;
            if(tempNextState == START_GAME)
                boardState.createMoveList(moves);
            else {
                // calculate next move
                //char desiredBoardState[8][8];
                //engine.makeMove(boardState.boardState, desiredBoardState);
                //boardState.createMoveList(moves, desiredBoardState);
            }
            // 8 bytes per move (2 bytes per value, 4 values)
            size_t messageLength = 1 + 8 * moves.size();
            output = new char[messageLength];
            output[0] = (char)currentFlags;
            for(int i = 0; i < moves.size(); i++) {
                // startX
                output[1+8*i] = (char)(moves[i].startX);
                output[1+8*i+1] = (char)(moves[i].startX >> 8);
                // startY
                output[1+8*i+2] = (char)(moves[i].startY);
                output[1+8*i+3] = (char)(moves[i].startY >> 8);
                // endX
                output[1+8*i+4] = (char)(moves[i].endX);
                output[1+8*i+5] = (char)(moves[i].endX >> 8);
                // endY
                output[1+8*i+6] = (char)(moves[i].endY);
                output[1+8*i+7] = (char)(moves[i].endY >> 8);
            }
            break;
    }
    return output;
}

char* FSM::runThread(char* flags, int& outputLen) {
    // update flags
    currentFlags = (int)(*flags);
    // update state
    nextState();
    char* output = outputState(outputLen);
    return output;
}
