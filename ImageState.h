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

#define REAL_IMG_OFFSET 1000
#define REAL_IMG_SQUARE_SIZE 100

struct CheckersPiece {
    int x;
    int y;
    int imageX;
    int imageY;
    bool onBoard;
    bool isBlue;
    bool isKing;
};

namespace std {
    bool operator==(const CheckersPiece& lhs, const CheckersPiece& rhs);
}

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
        // Returns whether coordinate is in correct range and converts coordinates to real space
        bool imageToRealSpace(int& realX, int& realY, int imgX, int imgY);
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
        std::list<CheckersPiece> redPiecesOnBoard;
        std::list<CheckersPiece> bluePiecesOnBoard;
        std::list<CheckersPiece> redPiecesOffBoard;
        std::list<CheckersPiece> bluePiecesOffBoard;
        std::list<CheckersPiece> redErrorPiecesOnBoard;
        std::list<CheckersPiece> blueErrorPiecesOnBoard;
        static const char STARTING_BOARD[8][8];
    private:
        // Turns cluster into piece
        void createPieceFromCluster(CheckersPiece& checker, Cluster& cluster);
        // Finds empyt spot off board to place piece
        bool findEmptySpotOffBoard(ImageMove& move, CheckersPiece& cp);
        // Find piece from off board
        bool findPieceFromOffBoard(ImageMove& move, IncorrectSquare& s, char piece);
        // Returns coordinates of square
        void getSquareCoords(int& imageX, int& imageY, int squareX, int squareY);
        // Add piece to board
        void addPieceToBoard(std::list<CheckersPiece> originalList, CheckersPiece& piece);
        void removeFromBoard(std::list<CheckersPiece> originalList, CheckersPiece& piece);
};

void printBoardState(const char boardState[8][8]);

#endif
