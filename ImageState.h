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
#include <string>
#include "Cluster.h"

struct CheckersPiece {
    int x;
    int y;
    bool onBoard;
    bool isBlue;
    bool isKing;
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
        bool generateBoardstate(cv::Mat& img);
        // Aligns camera to checkers board, returns true or false depending on if it worked
        bool alignCamera(cv::Mat& img);
        // Returns the row or column 
        cv::Point2i getBoardPos(CheckersPiece& p);
        std::vector<cv::Point2f> boardCorners;
        bool isValidState = false;
        // 0 is lower valued edge
        int edgeX[2];
        int edgeY[2];
        int avgSquareWidth;
        int avgSquareHeight;
        std::string boardState = "";
        std::string lastValidBoardState = "";
        std::vector<CheckersPiece> redPiecesOnBoard;
        std::vector<CheckersPiece> bluePiecesOnBoard;
        std::vector<CheckersPiece> redPiecesOffBoard;
        std::vector<CheckersPiece> bluePiecesOffBoard;
    private:
        // Turns cluster into piece
        void createPieceFromCluster(CheckersPiece& checker, Cluster& cluster);
};

#endif
