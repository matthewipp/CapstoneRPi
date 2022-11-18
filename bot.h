

#ifndef BOT_CLASS
#define BOT_CLASS

#include "defs.h"
#include <cstring>
#include <vector>
#include <string>
#include <cstdint>

class Bot {
public:

    typedef struct {
        uint8_t x;
        uint8_t y;
    } Coord;

    typedef struct {
        Bot::Coord s;     // Start x, y coordinates
        Bot::Coord f;     // End x, y coordinates
        char pm;        // Piece that made the move
        bool cap;       // Was move capture?
        char pe;        // Piece that was captured
        bool t;         // Does the turn continue?
    } Move;

    typedef struct {
        char b[8][8];
        bool cont;
        Bot::Coord p;
    } State;

    uint8_t depth;
    bool color;
    bool eval_color;
    volatile uint32_t node_count;
    char board[8][8];
    uint8_t red_count;
    uint8_t blue_count;
    Bot::Coord move_focus = {8, 8};
    bool focused = false;

    Bot();
    Bot(uint8_t depth);
    uint move();
    int alpha_beta(uint8_t, int, int);
    std::vector<Bot::Move> moves();
    int eval();
    bool over();
    void init_board();
    void init_board(std::string);
    void apply_move(Bot::Move);
    void undo_move(Bot::Move);
    Move calc_move(uint8_t, int, int);
    bool checkMoves(char, int8_t, Bot::Coord);
    void set_board(char b[8][8]);
    uint gen_move(char b[8][8]);
    bool board_equal(char b1[8][8], char b2[8][8]);
    bool comp_boards(char bi[8][8], char bf[8][8]);
    void init_state(Bot::State * pS, char b[8][8], bool cont, Bot::Coord p);
    
};

#endif
