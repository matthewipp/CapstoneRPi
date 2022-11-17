

#ifndef BOT_CLASS
#define BOT_CLASS

#include "defs.h"
#include <vector>
#include <string>

typedef struct {
    uchar s[2];     // Start x, y coordinates
    uchar f[2];     // End x, y coordinates
    char pm;        // Piece that made the move
    bool cap;       // Was move capture?
    char pe;        // Piece that was captured
    bool t;         // Does the turn continue?
} Move;

class Bot {
public:
    uchar depth;
    bool color;
    bool eval_color;
    uint node_count;
    char board[8][8];
    uchar red_count;
    uchar blue_count;

    Bot();
    Bot(uchar depth);
    uint move();
    int alpha_beta(uchar, int, int);
    std::vector<Move> moves();
    int eval();
    bool over();
    void init_board();
    void init_board(std::string);
    void apply_move(Move);
    void undo_move(Move);
    Move calc_move(uchar, int, int);
    bool checkMoves(char, char, uchar, uchar);
    void set_board(char b[8][8]);
    void gen_move(char b[8][8]);
    bool board_equal(char b1[8][8], char b2[8][8]);
    bool comp_boards(char bi[8][8], char bf[8][8]);
    
};

#endif
