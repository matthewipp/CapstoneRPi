
#include <iostream>
#include "bot.h"

std::string board_to_string(char b[8][8]) {

    std::string ret = "";

    for (uchar y = 0; y < 8; y++) {
        for (uchar x = 0; x < 8; x++) {
            // std::cout << (int)x << ", " << (int)y << std::endl;
            // std::cout << (int)b[x][y] << std::endl;
            // std::cout << "good" << std::endl;
            if (b[x][y])
                ret = ret + b[x][y] + " ";
            else
                ret += ". ";
        }
        ret += "\n";
    }

    return ret;

}

// std::string move_to_string(Move m) {
//     return "(" + m.s[0] + ", " + m.s[1] + ") -> (" + m.f[0] + ", " + m.f[1] + ")";
// }

int main() {
    /*
    Bot b;
    b.init_board();
    // b.board[2][3] = 'b';
    b.board[5][3] = 'R';
    b.board[4][2] = 'b';
    b.board[4][4] = 'b';
    b.board[6][2] = 'B';
    b.board[6][4] = 'b';



    std::cout << "Hello" << std::endl;

    std::cout << board_to_string(b.board) << std::endl;

    std::cout << (int) b.blue_count << std::endl;

    for (Move m : b.moves()) {
        std::cout << PRINT_MOVE(m) << '\n' << std::endl;
    }

    Move m = b.moves()[0];

    b.apply_move(m);

    std::cout << board_to_string(b.board) <<  std::endl;

    std::cout << (int) b.blue_count << std::endl;

    b.undo_move(m);

    std::cout << board_to_string(b.board) <<  std::endl;

    std::cout << (int) b.blue_count << std::endl;

    std::cout << b.move() << std::endl;

    std::cout << board_to_string(b.board) <<  std::endl;

    std::cout << '\n' << '\n' <<  std::endl;
    */

    Bot g;
    Bot q;
    Bot c;
    q.bot_color = false;
    q.max_depth = 10;
    g.max_depth = 6;
    q.evalFunc = &Bot::evalII;
    g.evalFunc = &Bot::evalI;
    bool blue_first = false;

    q.init_board();
    // q.init_board("0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 R 0 0 0 0 r 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 B 0 0 0 0 0 0 0 0 0 0 R 0 0 B 0 0 0 0 0");
    // q.init_board("0 b 0 0 0 0 0 R r 0 0 0 0 0 0 0 0 0 0 0 0 0 0 R 0 0 0 0 0 0 R 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 R 0 0 0 0 0 0 0 0 R 0");

    g.set_board(q.board);
    std::cout << board_to_string(q.board) <<  std::endl;
    uint32_t bnum, rnum = bnum = 0;
    while (!g.over() || !q.over()) {
        if (!blue_first) {
            rnum = std::max(g.gen_move(q.board), rnum);
            c.bot_color = q.bot_color;
            std::cout << "Legal Move: " << c.comp_boards(q.board, g.board) << std::endl;
            std::cout << "Board Equal: " << c.board_equal(g.board, q.board) << std::endl;
            std::cout << board_to_string(g.board) <<  std::endl;
            std::cout << "G's Eval: " << g.current_eval << " (" << (int)g.red_count << ", " << (int)g.blue_count << ") " << rnum << '\n' << std::endl;
        }
        bnum = std::max(bnum, q.gen_move(g.board));
        c.bot_color = g.bot_color;
        std::cout << "Legal Move: " << c.comp_boards(g.board, q.board) << std::endl;
        std::cout << board_to_string(q.board) << std::endl;
        std::cout << "Q's Eval: " << q.current_eval << " (" << (int)q.red_count << ", " << (int)q.blue_count << ") " << bnum << '\n' << std::endl;
        blue_first = false;
        // break;
    }
    // std::cout << board_to_string(g.board) <<  std::endl;

    // std::cout << q.gen_move(g.board) << '\n';

    // std::cout << board_to_string(q.board) <<  std::endl;

    // while (!g.over()) {
    //     std::cout << g.move() << std::endl;
    //     std::cout << board_to_string(g.board) <<  std::endl;
    //     std::cout << std::endl;
    // }

    // std::cout << "Done" << std::endl;

    // std::cout << g.move() << std::endl;

    // std::cout << board_to_string(g.board) <<  std::endl;

    return 0;
}
