

#ifndef BOT_CLASS
#define BOT_CLASS

#include "defs.h"
#include <cstring>
#include <vector>
#include <string>

class Bot {
public:

    typedef struct {
        uchar x;
        uchar y;
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

    uchar depth;
    bool color;
    bool eval_color;
    uint node_count;
    char board[8][8];
    uchar red_count;
    uchar blue_count;
    Bot::Coord move_focus;

    Bot();
    Bot(uchar depth);
    uint move();
    int alpha_beta(uchar, int, int);
    std::vector<Bot::Move> moves();
    int eval();
    bool over();
    void init_board();
    void init_board(std::string);
    void apply_move(Bot::Move);
    void undo_move(Bot::Move);
    Move calc_move(uchar, int, int);
    bool checkMoves(char, char, uchar, uchar);
    void set_board(char b[8][8]);
    uint gen_move(char b[8][8]);
    bool board_equal(char b1[8][8], char b2[8][8]);
    bool comp_boards(char bi[8][8], char bf[8][8]);
    void init_state(Bot::State * pS, char b[8][8], bool cont, Bot::Coord p);
    
};

#endif
