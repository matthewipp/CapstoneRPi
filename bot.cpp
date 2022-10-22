
#include "bot.h"
#include <iostream>

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
Function that takes board state and generates move.
Note: move is represented as an update to the board
member variable
*/
uint Bot::gen_move(char b[8][8]) {
    // Set the board state to match the argument
    this->set_board(b);
    // Set Red to move
    this->color = true;
    std::cout << "Number of legal moves: " << this->moves().size() << std::endl;
    uint32_t sum = 0;
    while (this->color) {
        std::cout << "In loop" << std::endl;
        sum += this->move();
    }
    // Generate the move
    return sum;
}

/*
Function for applying Move to board
*/
void Bot::apply_move(Bot::Move m) {
    // Check if piece should be promoted and red
    if (this->color && m.f.y == 7 && !IS_CROWNED(this->board[m.s.x][m.s.y])) {
        this->board[m.s.x][m.s.y] = 'R';
    }
    // Check if piece should be promoted and blue
    if (!this->color && m.f.y == 0 && !IS_CROWNED(this->board[m.s.x][m.s.y])) {
        this->board[m.s.x][m.s.y] = 'B';
    }

    if (!m.cap) {   // If the move is not a capture
        this->board[m.f.x][m.f.y] = this->board[m.s.x][m.s.y];
        this->board[m.s.x][m.s.y] = 0;
        this->color = !this->color;
    } else {        // If the move is a capture
        if (this->color) 
            blue_count--;
        else 
            red_count--;
        this->board[m.f.x][m.f.y] = this->board[m.s.x][m.s.y];
        this->board[m.s.x][m.s.y] = 0;
        this->board[(m.s.x+m.f.x)>>1][(m.s.y+m.f.y)>>1] = 0;
        this->color = m.t ? this->color : !this->color;
    }
}

/*
Function to set the board state
*/
void Bot::set_board(char b[8][8]) {
    // Initialize red and blue piece counts
    this->red_count = this->blue_count = 0;
    // Iterate through the board
    for (uint8_t x = 0; x < 8; x++) {
        for (uint8_t y = 0; y < 8; y++) {
            // If there's a piece at this tile, increment the corresponding color count
            if (b[x][y] != 0) {
                if (IS_RED(b[x][y])) {
                    this->red_count++;
                } else {
                    this->blue_count++;
                }
            }
            // Update the board member variable to match the argument
            this->board[x][y] = b[x][y];
        }
    }
}

/*
Compares two 2D character arrays
*/
bool Bot::board_equal(char b1[8][8], char b2[8][8]) {
    // Iterate through every element
    for (uint8_t x = 0; x < 8; x++) {
        for (uint8_t y = 0; y < 8; y++) {
            // If the elements do not match, return false
            if (b1[x][y] != b2[x][y]) {
                return false;
            }
        }
    }
    // If all elements match, return true
    return true;
}

/*
Function to initialize State struct
*/
void Bot::init_state(Bot::State * pS, char b[8][8], bool cont, Bot::Coord p) {
    // Copy 8x8 char array
    std::memcpy(pS->b, b, sizeof(b));
    // Assign cont
    pS->cont = cont;
    // Copy uchar array
    // std::memcpy(ret.p, p, sizeof(p));
    pS->p = p;
}

/*
Function to determine if bf is reachable from bi in a 
single legal move.
Note: one legal move can consist of multiple "Move" structs
as long as they don't cede the turn.
*/
bool Bot::comp_boards(char bi[8][8], char bf[8][8]) {
    // Prepare stack for depth first traversal
    std::vector<Bot::State*> stack;
    Bot::State * s = new State();
    Bot::State * temp;
    init_state(s, bi, false, {0, 0});
    stack.push_back(s);

    // Depth first traversal
    while (stack.size()) {
        // Get back element
        s = stack.back();
        stack.pop_back();
        // Configure board state
        this->set_board(s->b);
        // Set as blue's turn
        this->color = false;
        // Check each move that can be applied on this state
        for (Move m : this->moves()) {
            // Try move
            this->apply_move(m);
            // Check if end state has been reached
            if (this->board_equal(this->board, bf)) {
                return true;
            }
            // If move continues, add subsequent board state to stack
            if (!this->color) {
                temp = new State();
                init_state(temp, this->board, true, m.f);
                stack.push_back(temp);
            }
        }
        delete s;
    }
    // If traversal completes, final state was unreachable
    return false;
}

/*
Function for undoing previously applied Move (requires previous move as argument)
*/
void Bot::undo_move(Bot::Move m) {
    if (!m.cap) {   // If the move is not a capture
        this->board[m.s.x][m.s.y] = m.pm;
        this->board[m.f.x][m.f.y] = 0;
        this->color = !this->color;
    } else {        // If the move is a capture
        this->board[m.s.x][m.s.y] = m.pm;
        this->board[m.f.x][m.f.y] = 0;
        this->board[(m.s.x+m.f.x)>>1][(m.s.y+m.f.y)>>1] = m.pe;
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
Bot::Move Bot::calc_move(uint8_t depth, int alpha, int beta) {
    int best_value;
    int value;
    Bot::Move best_move;
    if (this->color) {
        best_value = -MAX_INT;
        for (Move move : this->moves()) {
            if (move.t) {
                this->move_focus = move.f;
                this->focused = true;
            }
            this->apply_move(move);
            value = this->alpha_beta(depth, alpha, beta);
            this->undo_move(move);
            if (move.t) {
                this->focused = false;
            }
            if (value >= best_value) {
                best_move = move;
                best_value = value;
            }
        }
    } else {
        best_value = MAX_INT;
        for (Move move : this->moves()) {
            if (move.t) {
                this->move_focus = move.f;
                this->focused = true;
            }
            this->apply_move(move);
            value = this->alpha_beta(depth, alpha, beta);
            this->undo_move(move);
            if (move.t) {
                this->focused = false;
            }
            if (value <= best_value) {
                best_move = move;
                best_value = value;
            }
        }
    }
    // std::cout << "Board Eval: " << best_value << std::endl;
    return best_move;
}

int Bot::alpha_beta(uint8_t depth, int alpha, int beta) {
    this->node_count++;
    if (depth == 0 || this->over()) {   // If game is over or if leaf node
        return this->eval();
    }
    if (this->color) {                  // If the current evaluation is from red perspective
        int value = -MAX_INT;
        for (Move move : this->moves()) {
            if (move.t) {
                this->move_focus = move.f;
                this->focused = true;
            }
            this->apply_move(move);
            value = max(value, this->alpha_beta(depth-1, alpha, beta));
            this->undo_move(move);
            if (move.t) {
                this->focused = false;
            }
            alpha = max(alpha, value);
            if (value >= beta) 
                break;
        }
        return value;
    } else {                            // If the current evaluation is from blue perspective
        int value = MAX_INT;
        for (Move move : this->moves()) {
            if (move.t) {
                this->move_focus = move.f;
                this->focused = true;
            }
            this->apply_move(move);
            value = min(value, this->alpha_beta(depth-1, alpha, beta));
            this->undo_move(move);
            if (move.t) {
                this->focused = false;
            }
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
    int score = 0;
    if (this->over()) {
        return this->red_count > this->blue_count ? MAX_INT : -MAX_INT;
    }
    for (uint8_t x = 0; x < 8; x++) {
        for (uint8_t y = 0; y < 8; y++) {
            if (this->board[x][y] == 0)
                continue;
            if (IS_RED(this->board[x][y]))
                score += IS_CROWNED(this->board[x][y]) ? 2 : 1;
            else
                score -= IS_CROWNED(this->board[x][y]) ? 2 : 1;
        }
    }
    return score;
}

/*
Default constructor for Bot type
*/
Bot::Bot() {

    this->color = true;

    for (uint8_t x = 0; x < 8; x++) {
        for (uint8_t y = 0; y < 8; y++) {
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

    for (uint8_t x = 0; x < 8; x++) {
        for (uint8_t y = 0; y < 3; y++)
            if ((x + y) % 2 == 1)
                this->board[x][y] = 'r';
        for (uint8_t y = 5; y < 8; y++)
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
    for (uint8_t x = 0; x < 8; x++) {
        for (uint8_t y = 0; y < 8; y++) {
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

bool Bot::checkMoves(char p, int8_t dir, Bot::Coord pos) {

    uint8_t x = pos.x;
    uint8_t y = pos.y;

    for (int8_t yDiff = -1; yDiff <= 1; yDiff += 2) {
        if (yDiff > dir + 1 || yDiff < dir - 1) 
            continue;
        for (int8_t xDiff = -1; xDiff <= 1; xDiff += 2) {
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
std::vector<Bot::Move> Bot::moves() {
    // Tracks which direction piece can move (along y-axis)
    int8_t dir;
    // Tracks whether it is possible to capture
    bool can_cap = false;
    // Stores return value
    std::vector<Move> ret;
    if (!focused) {
        // Loop through every tile on the board
        for (uint8_t x = 0; x < 8; x++) {
            for (uint8_t y = 0; y < 8; y++) {
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
                
                for (int8_t yDiff = -1; yDiff <= 1; yDiff += 2) {
                    if (yDiff > dir + 1 || yDiff < dir - 1) 
                        continue;
                    for (int8_t xDiff = -1; xDiff <= 1; xDiff += 2) {
                        if (y + (yDiff<<1) <= 7 && y + (yDiff<<1) >= 0) {
                            if (x + (xDiff<<1) <= 7 && x + (xDiff<<1) >= 0) {
                                if (board[x+xDiff][y+yDiff] != 0 && IS_RED(board[x+xDiff][y+yDiff]) != IS_RED(board[x][y])) {
                                    if (board[x+(xDiff<<1)][y+(yDiff<<1)] == 0) {
                                        if (!can_cap) {
                                            can_cap = true;
                                            ret.clear();
                                        }
                                        ret.push_back({{x, y}, {(uint8_t)(x+(xDiff<<1)), (uint8_t)(y+(yDiff<<1))}, 
                                                        board[x][y], true, board[x+xDiff][y+yDiff], 
                                                        checkMoves(board[x][y], dir, {(uint8_t)(x+(xDiff<<1)), (uint8_t)(y+(yDiff<<1))})});
                                    }
                                }
                            }
                        }
                        if (!can_cap) {
                            if (x + xDiff <= 7 && x + xDiff >= 0) {
                                if (y + yDiff <= 7 && y + yDiff >= 0) {
                                    if (board[x+xDiff][y+yDiff] == 0) {
                                        ret.push_back({{x, y}, {(uint8_t)(x+xDiff), (uint8_t)(y+yDiff)}, 
                                                    board[x][y], false, 0, false});
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        return ret;
    } else {
        uint8_t x = this->move_focus.x;
        uint8_t y = this->move_focus.y;
        // Skip if the square is empty
        if (board[x][y] == 0) 
            return ret;
        // Skip if the piece is the wrong color
        if (IS_RED(board[x][y]) != color) 
            return ret;
        // If piece is crowned allow it to move in both directions; else, direction is determined by color
        if (IS_CROWNED(board[x][y]))
            dir = 0;
        else
            dir = this->color ? RED_DIR : BLUE_DIR;
        
        for (int8_t yDiff = -1; yDiff <= 1; yDiff += 2) {
            if (yDiff > dir + 1 || yDiff < dir - 1) 
                continue;
            for (int8_t xDiff = -1; xDiff <= 1; xDiff += 2) {
                if (y + (yDiff<<1) <= 7 && y + (yDiff<<1) >= 0) {
                    if (x + (xDiff<<1) <= 7 && x + (xDiff<<1) >= 0) {
                        if (board[x+xDiff][y+yDiff] != 0 && IS_RED(board[x+xDiff][y+yDiff]) != IS_RED(board[x][y])) {
                            if (board[x+(xDiff<<1)][y+(yDiff<<1)] == 0) {
                                if (!can_cap) {
                                    can_cap = true;
                                    ret.clear();
                                }
                                ret.push_back({{x, y}, {(uint8_t)(x+(xDiff<<1)), (uint8_t)(y+(yDiff<<1))}, 
                                                board[x][y], true, board[x+xDiff][y+yDiff], 
                                                checkMoves(board[x][y], dir, {(uint8_t)(x+(xDiff<<1)), (uint8_t)(y+(yDiff<<1))})});
                            }
                        }
                    }
                }
                if (!can_cap) {
                    if (x + xDiff <= 7 && x + xDiff >= 0) {
                        if (y + yDiff <= 7 && y + yDiff >= 0) {
                            if (board[x+xDiff][y+yDiff] == 0) {
                                ret.push_back({{x, y}, {(uint8_t)(x+xDiff), (uint8_t)(y+yDiff)}, 
                                            board[x][y], false, 0, false});
                            }
                        }
                    }
                }
            }
        }
        this->focused = false;
        return ret;
    }
}
