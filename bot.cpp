
#include "bot.h"
#include <cstring>

/*
Function for bot performing move
*/
uint Bot::move() {
    // Reset node count to track number of nodes processed in this step
    node_count = 0;
    // Apply the calculated move (search with depth of 10)
    this->apply_move(this->calc_move(10, -MAX_INT, MAX_INT));
    // Return the node count
    return node_count;
}

/*
Function that takes board state and generates move
*/
void Bot::gen_move(char b[8][8]) {
    this->set_board(b);
    this->color = true;
    this->move();
}

/*
Function for applying Move to board
*/
void Bot::apply_move(Move m) {
    // Check if piece should be promoted and red
    if (this->color && m.f[1] == 7 && !IS_CROWNED(this->board[m.s[0]][m.s[1]])) {
        this->board[m.s[0]][m.s[1]] = 'R';
    }
    // Check if piece should be promoted and blue
    if (!this->color && m.f[1] == 0 && !IS_CROWNED(this->board[m.s[0]][m.s[1]])) {
        this->board[m.s[0]][m.s[1]] = 'B';
    }

    if (!m.cap) {   // If the move is not a capture
        this->board[m.f[0]][m.f[1]] = this->board[m.s[0]][m.s[1]];
        this->board[m.s[0]][m.s[1]] = 0;
        this->color = !this->color;
    } else {        // If the move is a capture
        if (this->color) 
            blue_count--;
        else 
            red_count--;
        this->board[m.f[0]][m.f[1]] = this->board[m.s[0]][m.s[1]];
        this->board[m.s[0]][m.s[1]] = 0;
        this->board[(m.s[0]+m.f[0])>>1][(m.s[1]+m.f[1])>>1] = 0;
        this->color = m.t ? this->color : !this->color;
    }
}

/*
Function to set the board state
*/
void Bot::set_board(char b[8][8]) {
    this->red_count = this->blue_count = 0;
    for (uchar x = 0; x < 8; x++) {
        for (uchar y = 0; y < 8; y++) {
            if (b[x][y] != 0) {
                if (IS_RED(b[x][y])) {
                    this->red_count++;
                } else {
                    this->blue_count++;
                }
            }
            this->board[x][y] = b[x][y];
        }
    }
}

/*
Compares two 2D character arrays
*/
bool Bot::board_equal(char b1[8][8], char b2[8][8]) {
    for (uchar x = 0; x < 8; x++) {
        for (uchar y = 0; y < 8; y++) {
            if (b1[x][y] != b2[x][y]) {
                return false;
            }
        }
    }
    return true;
}

State Bot::init_state(char b[8][8], bool cont, uchar p[2]) {
    State ret;
    std::memcpy(ret.b, b, sizeof(b));
    ret.cont = cont;
    std::memcpy(ret.p, p, sizeof(p));
    return ret;
}

bool Bot::comp_boards(char bi[8][8], char bf[8][8]) {
    std::vector<State> stack;
    uchar p[2] = {0, 0};
    State s = init_state(bi, false, p);
    stack.push_back(s);
    while (stack.size()) {
        s = stack.back();
        stack.pop_back();
        this->set_board(s.b);
        this->color = false;
        for (Move m : this->moves()) {
            this->apply_move(m);
            if (this->board_equal(this->board, bf)) {
                return true;
            }
            if (!this->color) {
                return false;
            }
        }
    }
    return false;
}

/*
bool Bot::comp_boards(char bi[8][8], char bf[8][8]) {
    std::vector<char[8][8]> stack;
    char state[8][8];
    while (stack.size()) {
        std::memcpy(state, stack.back(), sizeof(stack.back()));
        stack.pop_back();
        this->set_board(state);
        this->color = false;
        for (Move m : this->moves()) {
            this->apply_move(m);
            if (this->board_equal(this->board, bf)) {
                return true;
            }
            if (!this->color) {
                return false;
            }
        }
    }
    return false;
}
*/

/*
Function for undoing previously applied Move (requires previous move as argument)
*/
void Bot::undo_move(Move m) {
    if (!m.cap) {   // If the move is not a capture
        this->board[m.s[0]][m.s[1]] = m.pm;
        this->board[m.f[0]][m.f[1]] = 0;
        this->color = !this->color;
    } else {        // If the move is a capture
        this->board[m.s[0]][m.s[1]] = m.pm;
        this->board[m.f[0]][m.f[1]] = 0;
        this->board[(m.s[0]+m.f[0])>>1][(m.s[1]+m.f[1])>>1] = m.pe;
        this->color = m.t ? this->color : !this->color;
        if (this->color)
            blue_count++;
        else 
            red_count++;
    }
}

/*
Function returns the max of two ints
*/
int max(int a, int b) {
    return a > b ? a : b;
}

/*
Function returns the min of two ints
*/
int min(int a, int b) {
    return a < b ? a : b;
}

/*
Function calculates algorithms chosen move
*/
Move Bot::calc_move(uchar depth, int alpha, int beta) {
    int best_value;
    int value;
    Move best_move;
    if (this->color) {
        best_value = -MAX_INT;
        for (Move move : this->moves()) {
            this->apply_move(move);
            value = this->alpha_beta(depth, alpha, beta);
            this->undo_move(move);
            if (value > best_value) {
                best_move = move;
                best_value = value;
            }
        }
    } else {
        best_value = MAX_INT;
        for (Move move : this->moves()) {
            this->apply_move(move);
            value = this->alpha_beta(depth, alpha, beta);
            this->undo_move(move);
            if (value < best_value) {
                best_move = move;
                best_value = value;
            }
        }
    }
    return best_move;
}

int Bot::alpha_beta(uchar depth, int alpha, int beta) {
    if (depth == 0 || this->over()) {   // If game is over or if leaf node
        return this->eval();
    }
    if (this->color) {                  // If the current evaluation is from red perspective
        int value = -MAX_INT;
        for (Move move : this->moves()) {
            this->apply_move(move);
            value = max(value, this->alpha_beta(depth-1, alpha, beta));
            this->undo_move(move);
            alpha = max(alpha, value);
            if (value >= beta) 
                break;
        }
        return value;
    } else {                            // If the current evaluation is from blue perspective
        int value = MAX_INT;
        for (Move move : this->moves()) {
            this->apply_move(move);
            value = min(value, this->alpha_beta(depth-1, alpha, beta));
            this->undo_move(move);
            beta = min(beta, value);
            if (value <= alpha)
                break;
        }
        return value;
    }
}

/*
Function determines if game is over
*/
bool Bot::over() {
    return this->red_count == 0 || this->blue_count == 0;
}

/*
Function evaluates board
*/
int Bot::eval() {
    this->node_count++;
    int score = 0;
    if (this->over()) {
        return this->red_count > this->blue_count ? MAX_INT : -MAX_INT;
    }
    for (uchar x = 0; x < 8; x++) {
        for (uchar y = 0; y < 8; y++) {
            if (this->board[x][y] == 0)
                continue;
            if (IS_RED(this->board[x][y]))
                score += IS_CROWNED(this->board[x][y]) ? 2 : 1;
            else
                score -= IS_CROWNED(this->board[x][y]) ? 2 : 1;
            // score += (2*IS_RED(this->board[x][y])-1)*(IS_CROWNED(this->board[x][y]) ? 2 : 1);
        }
    }
    return score;
}

/*
Default constructor for Bot type
*/
Bot::Bot() {

    this->color = true;

    for (uchar x = 0; x < 8; x++) {
        for (uchar y = 0; y < 8; y++) {
            this->board[x][y] = 0;
        }
    }

    this->red_count = 8;
    this->blue_count = 8;

}

/*
Default board initialization function
Configures start position
*/
void Bot::init_board() {

    for (uchar x = 0; x < 8; x++) {
        for (uchar y = 0; y < 3; y++)
            if ((x + y) % 2 == 1)
                this->board[x][y] = 'r';
        for (uchar y = 5; y < 8; y++)
            if ((x + y) % 2 == 1)
                this->board[x][y] = 'b';
    }

}

/*
Alternative board initialization function
Arbitrary board setup determined by string input
*/
void Bot::init_board(std::string bState) {
    this->red_count = this->blue_count = 0;
    std::string tok;
    for (uchar x = 0; x < 8; x++) {
        for (uchar y = 0; y < 8; y++) {
            tok = bState.substr(0, bState.find(" "));
            bState = bState.substr(bState.find(" ") + 1, bState.size());
            if (tok == "0")
                this->board[x][y] = 0;
            else {
                this->board[x][y] = tok[0];
                if (IS_RED(tok[0]))
                    this->red_count++;
                else 
                    this->blue_count++;
            }
        }
    }
}

/*

bool Bot::checkMoves(char p, char dir, bool can_cap, uchar x, uchar y, Move ret[], bool cont = true) {
    char ind = 0;
    bool retFlag = false;

    for (char yDiff = -1; yDiff <= 1; yDiff += 2) {
        if (diff > dir + 1 || diff < dir - 1) 
            continue;
        for (char xDiff = -1; xDiff <= 1; xDiff += 2) {
            if (!can_cap) {
                if (y + yDiff <= 7 && y + yDiff >= 0) {
                    if (x + xDiff <= 7 && x + xDiff >= 0) {
                        if (board[x+xDiff][y+diff] == 0) {
                            ret[ind] = {{x, y}, {x+xDiff, y+yDiff}, p, false, 0, false};
                            retFlag = true;
                        }
                    }
                }
            }
            if (y + (yDiff<<1) <= 7 && y + (yDiff<<1) >= 0) {
                if (x + (xDiff<<1) <= 7 && x + (xDiff<<1) >= 0) {
                    if (board[x+xDiff][y+yDiff] != 0 && IS_RED(board[x+xDiff][y+yDiff]) != IS_RED(p)) {
                        if (board[x+(xDiff<<1)][y+(yDiff<<1)] == 0) {
                            ret[ind] = {{x, y}, {x+(xDiff<<1), y+(yDiff<<1)}, p, false, 0, false};
                        }
                    }
                }
            }
        }
    }
}

*/

bool Bot::checkMoves(char p, char dir, uchar x, uchar y) {
    char ind = 0;
    bool retFlag = false;

    for (char yDiff = -1; yDiff <= 1; yDiff += 2) {
        if (yDiff > dir + 1 || yDiff < dir - 1) 
            continue;
        for (char xDiff = -1; xDiff <= 1; xDiff += 2) {
            if (y + (yDiff<<1) <= 7 && y + (yDiff<<1) >= 0) {
                if (x + (xDiff<<1) <= 7 && x + (xDiff<<1) >= 0) {
                    if (board[x+xDiff][y+yDiff] != 0 && IS_RED(board[x+xDiff][y+yDiff]) != IS_RED(p)) {
                        if (board[x+(xDiff<<1)][y+(yDiff<<1)] == 0) {
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

/*
Calculates list of legal moves from current board state
*/
std::vector<Move> Bot::moves() {
    
    // Tracks which direction piece can move (along y-axis)
    char dir;
    // Tracks whether it is possible to capture
    bool can_cap = false;
    // Stores return value
    std::vector<Move> ret;
    // Turn continues
    bool turn_continues = false;

    // Loop through every tile on the board
    for (uchar x = 0; x < 8; x++) {
        for (uchar y = 0; y < 8; y++) {
            // Skip if the square is empty
            if (board[x][y] == 0) 
                continue;
            // Skip if the piece is the wrong color
            if (IS_RED(board[x][y]) != color) 
                continue;

            // If piece is crowned allow it to move in both directions; else, direction is determined by color
            if (IS_CROWNED(board[x][y]))
                dir = 0;
            else
                dir = this->color ? RED_DIR : BLUE_DIR;
            
            // If it is capable of moving in -y direction, check for potential moves in that direction
            if (dir < 1) {
                // Ensure that it has room to move in the -y direction
                if (y > 0) {
                    // If it can capture, no need to check moves that won't result in capture
                    if (!can_cap) {
                        // Check if diagonal-adjacent tiles are empty
                        if (x < 7 && (board[x+1][y-1] == 0)) {
                            ret.push_back({{x, y}, {(uchar)(x+1), (uchar)(y-1)}, board[x][y], false, 0, false});
                        }
                        if (x > 0 && (board[x-1][y-1] == 0)) {
                            ret.push_back({{x, y}, {(uchar)(x-1), (uchar)(y-1)}, board[x][y], false, 0, false});
                        }
                    }
                    // Capturing requires more space
                    if (y > 1) {
                        // Check if enemy piece is diagonal-adjacent with an opening after
                        if (x < 6 && board[x+1][y-1] != 0 && IS_RED(board[x+1][y-1]) != this->color && board[x+2][y-2] == 0) {
                            // If this is the first capture, set capture flag and clear move list
                            if (!can_cap) {
                                can_cap = true;
                                ret.clear();
                            }
                            ret.push_back({{x, y}, {(uchar)(x+2), (uchar)(y-2)}, board[x][y], true, board[x+1][y-1], checkMoves(board[x][y], dir, x+2, y-2)});
                        }
                        if (x > 1 && board[x-1][y-1] != 0 && IS_RED(board[x-1][y-1]) != this->color && board[x-2][y-2] == 0) {
                            if (!can_cap) {
                                can_cap = true;
                                ret.clear();
                            }
                            ret.push_back({{x, y}, {(uchar)(x-2), (uchar)(y-2)}, board[x][y], true, board[x-1][y-1], checkMoves(board[x][y], dir, x-2, y-2)});
                        }
                    }
                }
            }

            // If it is capable of moving in +y direction, check for potential moves in that direction
            // analogous to above code for -y direction
            if (dir > -1) {
                if (y < 7) {
                    if (!can_cap) {
                        if (x < 7 && (board[x+1][y+1] == 0)) {
                            ret.push_back({{x, y}, {(uchar)(x+1), (uchar)(y+1)}, board[x][y], false, 0, false});
                        }
                        if (x > 0 && (board[x-1][y+1] == 0)) {
                            ret.push_back({{x, y}, {(uchar)(x-1), (uchar)(y+1)}, board[x][y], false, 0, false});
                        }
                    }
                    if (y < 6) {
                        if (x < 6 && board[x+1][y+1] != 0 && IS_RED(board[x+1][y+1]) != this->color && board[x+2][y+2] == 0) {
                            if (!can_cap) {
                                can_cap = true;
                                ret.clear();
                            }
                            ret.push_back({{x, y}, {(uchar)(x+2), (uchar)(y+2)}, board[x][y], true, board[x+1][y+1], checkMoves(board[x][y], dir, x+2, y+2)});
                        }
                        if (x > 1 && board[x-1][y+1] != 0 && IS_RED(board[x-1][y+1]) != this->color && board[x-2][y+2] == 0) {
                            if (!can_cap) {
                                can_cap = true;
                                ret.clear();
                            }
                            ret.push_back({{x, y}, {(uchar)(x-2), (uchar)(y+2)}, board[x][y], true, board[x-1][y+1], checkMoves(board[x][y], dir, x-2, y+2)});
                        }
                    }
                }
            }
        }
    }

    return ret;

}
