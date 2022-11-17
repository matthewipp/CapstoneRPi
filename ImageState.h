/**
 * @file ImageState.h
 * @author EMNEM
 * @brief Constains the ImageState class and CheckersPiece struct used for piece recog
 * @version 0.1
 * @date 2022-10-19
 */

#ifndef IMAGE_STATE_H
#define IMAGE_STATE_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <list>
#include <string>
#include "Cluster.h"

#define LEFT_COLS 1
#define RIGHT_COLS 2
#define TOP_ROWS 1
#define BOT_ROWS 2

struct CheckersPiece {
    int x;
    int y;
    int imageX;
    int imageY;
    bool onBoard;
    bool isBlue;
    bool isKing;
};

struct IncorrectSquare {
    int x;
    int y;
    CheckersPiece* occupiedPiece;
    CheckersPiece* matchedPiece;
};

struct ImageMove {
    int startX, startY;
    int endX, endY;
};

class ImageState {
    public:
        // Returns the number of red kings on the board
        int countRedKingsOnBoard();
        // Returns the number of blue kings on the board
        int countBlueKingsOnBoard();
        // Returns the number of red kings off the board
        int countRedKingsOffBoard();
        // Returns the number of blue kings off the board
        int countBlueKingsOffBoard();
        // Creates the board state from a bunch of clusters
        bool generateBoardState(std::vector<Cluster>& redClusters, std::vector<Cluster>& blueClusters);
        // changes the board state to match the image
        // The state still changes even if false is returned
        bool generateBoardstate(cv::Mat& img, bool checkLegalMove = true);
        // Aligns camera to checkers board, returns true or false depending on if it worked
        bool alignCamera(cv::Mat& img);
        // Returns the row or column 
        cv::Point2i getBoardPos(CheckersPiece& p);
        // Returns moves to create starting boardState
        void createMoveList(std::list<ImageMove>& moveList);
        // Returns moves to create given boardstate
        void createMoveList(std::list<ImageMove>& moveList, const char desiredBoard[8][8]);
        std::vector<cv::Point2f> boardCorners;
        bool isValidState = false;
        // 0 is lower valued edge
        int edgeX[2];
        int edgeY[2];
        int avgSquareWidth;
        int avgSquareHeight;
        char boardState[8][8];
        //char proposedBoardState[8][8];
        CheckersPiece* boardStatePointer[8][8];
        //CheckersPiece* proposedBoardStatePointer[8][8];
        char lastValidBoardState[8][8];
        bool majorFault = false;
        std::vector<CheckersPiece> redPiecesOnBoard;
        std::vector<CheckersPiece> bluePiecesOnBoard;
        std::vector<CheckersPiece> redPiecesOffBoard;
        std::vector<CheckersPiece> bluePiecesOffBoard;
        std::vector<CheckersPiece> redErrorPiecesOnBoard;
        std::vector<CheckersPiece> blueErrorPiecesOnBoard;
        static const char STARTING_BOARD[8][8];
    private:
        // Turns cluster into piece
        void createPieceFromCluster(CheckersPiece& checker, Cluster& cluster);
        // Finds empyt spot off board to place piece
        bool findEmptySpotOffBoard(ImageMove& move, IncorrectSquare& s);
        // Find piece from off board
        bool findPieceFromOffBoard(ImageMove& move, IncorrectSquare& s, char piece);
        // Returns coordinates of square
        void getSquareCoords(int& imageX, int& imageY, int squareX, int squareY);
};

void printBoardState(const char boardState[8][8]);

#endif
