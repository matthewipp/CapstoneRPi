
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
                ret += "0 ";
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

    // g.init_board();
    g.init_board("0 r 0 0 0 b 0 b r 0 r 0 0 0 b 0 0 r 0 0 0 b 0 b r 0 r 0 0 0 b 0 0 r 0 0 0 b 0 b r 0 r 0 0 0 b 0 0 r 0 0 0 b 0 b r 0 r 0 0 0 b 0");
    
    Bot q;

    std::cout << board_to_string(g.board) <<  std::endl;

    std::cout << q.gen_move(g.board) << '\n';

    std::cout << board_to_string(q.board) <<  std::endl;

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
