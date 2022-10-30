/**
 * @file ImageState.cpp
 * @author EMNEM
 * @brief Implementation of ImageState class for piece recognition
 * @version 0.1
 * @date 2022-10-19
 */

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include "ImageState.h"
#include "Cluster.h"
#include "PieceRecognition.h"

int ImageState::countRedKingsOnBoard() {
    int counter = 0;
    for(CheckersPiece& piece : redPiecesOnBoard) {
        if(piece.isKing) {
            counter++;
        }
    }
    return counter;
}

int ImageState::countBlueKingsOnBoard() {
    int counter = 0;
    for(CheckersPiece& piece : bluePiecesOnBoard) {
        if(piece.isKing) {
            counter++;
        }
    }
    return counter;
}

int ImageState::countRedKingsOffBoard() {
    int counter = 0;
    for(CheckersPiece& piece : redPiecesOffBoard) {
        if(piece.isKing) {
            counter++;
        }
    }
    return counter;
}

int ImageState::countBlueKingsOffBoard() {
    int counter = 0;
    for(CheckersPiece& piece : bluePiecesOffBoard) {
        if(piece.isKing) {
            counter++;
        }
    }
    return counter;
}

bool ImageState::generateBoardstate(cv::Mat& img) {
    // Get points
    std::vector<std::vector<Point>> points;
    getPointsInImage(img, points);
    std::vector<Point> bluePoints = points.back();
    points.pop_back();
    std::vector<Point> redPoints = points.back();
    points.pop_back();
    // Clusterize
    std::vector<Cluster> blueClusters;
    clusterize(bluePoints, true, blueClusters);
    std::vector<Cluster> redClusters;
    clusterize(redPoints, false, redClusters);
    bool valid = generateBoardState(redClusters, blueClusters);
    isValidState = valid;
    // check if the move was legal here
    bool wasLegalMove = true;
    if(valid && wasLegalMove) {
        lastValidBoardState = boardState;
        return true;
    }
    else {
        return false;
    }
}

bool ImageState::generateBoardState(std::vector<Cluster>& redClusters, std::vector<Cluster>& blueClusters) {
    bool success = true;
    redPiecesOnBoard.clear();
    redPiecesOffBoard.clear();
    bluePiecesOnBoard.clear();
    bluePiecesOffBoard.clear();
    // Red Pieces
    for(Cluster& c : redClusters) {
        CheckersPiece cp;
        cp.isBlue = false;
        cp.isKing = c.isKing;
        cp.x = c.x;
        cp.y = c.y;
        cp.onBoard = edgeX[0] < cp.x && cp.x < edgeX[1] && edgeY[0] < cp.y && cp.y < edgeY[1];
        if(cp.onBoard)
            redPiecesOnBoard.push_back(cp);
        else
            redPiecesOffBoard.push_back(cp);
    }
    // Blue Pieces
    for(Cluster& c : blueClusters) {
        CheckersPiece cp;
        cp.isBlue = true;
        cp.isKing = c.isKing;
        cp.x = c.x;
        cp.y = c.y;
        cp.onBoard = edgeX[0] < cp.x && cp.x < edgeX[1] && edgeY[0] < cp.y && cp.y < edgeY[1];
        if(cp.onBoard)
            bluePiecesOnBoard.push_back(cp);
        else
            bluePiecesOffBoard.push_back(cp);
    }
    // Check positions on board
    char pos[8][8];
    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            pos[i][j] = '.';
        }
    }
    for(CheckersPiece& p : redPiecesOnBoard) {
        cv::Point2i coord = getBoardPos(p);
        if(coord.x == -1) {
            // Invaloid coordinate
            success = false;
        }
        else if(pos[coord.x][coord.y] == '.') {
            if(p.isKing) {
                pos[coord.x][coord.y] = 'R';
            } 
            else {
                pos[coord.x][coord.y] = 'r';
            }
        } 
        else {
            // Two pieces on same spot
            success = false;
        }
    }
    for(CheckersPiece& p : bluePiecesOnBoard) {
        cv::Point2i coord = getBoardPos(p);
        if(coord.x == -1) {
            // Invalid coordinate
            success = false;
        }
        if(pos[coord.x][coord.y] == '.') {
            if(p.isKing) {
                pos[coord.x][coord.y] = 'B';
            } 
            else {
                pos[coord.x][coord.y] = 'b';
            }
        } 
        else {
            // Two pieces on same spot
            success = false;
        }
    }
    // update boardsate string
    boardState = "";
    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            boardState = boardState + pos[i][j];
        }
        boardState = boardState + "\n";
    }
    return success;
}

bool ImageState::alignCamera(cv::Mat& img) {
    int cornerWidth = 7;
    int cornerHeight = 7;
    cv::Size boardSize(cornerWidth, cornerHeight);
    std::vector<cv::Point2f> corners;
    bool found = cv::findChessboardCorners(img, boardSize, corners, cv::CALIB_CB_ADAPTIVE_THRESH + cv::CALIB_CB_FAST_CHECK);
    if(!found) {
        // Could not find checkers board
        std::cout << "Did not find grid\n";
        return false;
    }
    /*
    // Undistort
    std::vector<cv::Point3f> objPoints;
    std::vector<std::vector<cv::Point3f>> objPointsOuter;
    std::vector<std::vector<cv::Point2f>> cornersOuter;
    for(int i = 0; i < 7; i++) {
        for(int j = 0; j < 7; j++) {
            objPoints.push_back(cv::Point3f((float)j*10, (float)i*10, 0.0f));
        }
    }
    objPointsOuter.push_back(objPoints);
    cornersOuter.push_back(corners);
    cv::Mat k, d;
    std::vector<cv::Mat> rvecs, tvecs;
    //cv::calibrateCamera(objPointsOuter, cornersOuter, img.size(), k, d, rvecs, tvecs);
    cv::Mat newCameraMat = cv::getOptimalNewCameraMatrix(k, d, img.size(), 1, img.size());
    cv::Mat newImage;
    cv::undistort(img, newImage, k, d);
    */
    // test orientation
    float testSum = 0;
    bool alongX = true;
    for(int i = 1; i < cornerWidth; i++) {
        testSum += corners[i].x - corners[i-1].x;
    }
    if(testSum < 50 && testSum > -50) {
        // Goes along y axis first
        alongX = false;
    }
    // Find grid size
    // Calculate average board width and height:
    float widthSum = 0;
    float heightSum = 0;
    float topSum = 0;
    float botSum = 0;
    float leftSum = 0;
    float rightSum = 0;
    if(alongX) {
        std::cout << "alongX true\n";
    }
    if(alongX) {
        for(int i = 0; i < boardSize.area(); i++) {
            // Check if corner is not on the right end
            if(i % 7 != 6) {
                widthSum += corners[i+1].x - corners[i].x;
            }
            else {
                rightSum += corners[i].x;
            }
            // Check if corner is on the left
            if(i % cornerWidth == 0) {
                leftSum += corners[i].x;
            }
            // Check if corner is not on the bottom
            if(i < boardSize.area() - cornerWidth) {
                heightSum += corners[i+cornerWidth].y - corners[i].y;     
            }
            else {
                botSum += corners[i].y;
            }
            // Check if corner is on the top
            if(i < cornerWidth) {
                topSum += corners[i].y;
            }
        }
    }
    else {
        for(int i = 0; i < boardSize.area(); i++) {
            // Check if corner is not on the right end
            if(i < boardSize.area() - cornerHeight) {
                widthSum += corners[i+cornerHeight].x - corners[i].x;
            }
            else {
                rightSum += corners[i].x;
            }
            // Check if corner is on the left
            if(i < cornerHeight) {
                leftSum += corners[i].x;
            }
            // Check if corner is not on the bottom
            if(i % cornerHeight != 6) {
                heightSum += corners[i+cornerWidth].y - corners[i].y;     
            }
            else {
                botSum += corners[i].y;
            }
            // Check if corner is on the top
            if(i % cornerHeight == 0) {
                topSum += corners[i].y;
            }
        }
    }
    // The number of heights and widths is 1 less than the number of corners found per edge
    float avgWidth = widthSum / 42;
    float avgHeight = heightSum / 42;
    avgSquareHeight = (int)avgWidth;
    avgSquareWidth = (int)avgHeight;
    if(avgSquareHeight < 0) {
        avgSquareHeight *= -1;
    }
    if(avgSquareWidth < 0) {
        avgSquareWidth *= -1;
    }
    // Extrapolate out to board edges, x and y from coordinates to board are switched
    int lr1 = (int)(leftSum / cornerHeight - avgWidth);
    int lr2 = (int)(rightSum / cornerHeight + avgWidth);
    int tb1 = (int)(topSum / cornerWidth - avgHeight);
    int tb2 = (int)(botSum / cornerWidth + avgHeight);
    if(lr1 < lr2) {
        edgeY[0] = lr1;
        edgeY[1] = lr2;
    }
    else {
        edgeY[0] = lr2;
        edgeY[1] = lr1;
    }
    if(tb1 < tb2) {
        edgeX[0] = tb1;
        edgeX[1] = tb2;
    }
    else {
        edgeX[0] = tb2;
        edgeX[1] = tb1;
    }

    return true;
}

cv::Point2i ImageState::getBoardPos(CheckersPiece& p) {
    // Keep moving right or down until you get past the point
    cv::Point2i pos(0, 0);
    while(p.x > edgeX[0] + (pos.x+1)*avgSquareWidth) {
        pos.x++;
    }
    while(p.y > edgeY[0] + (pos.y+1)*avgSquareHeight) {
        pos.y++;
    }
    // Check distance, if too far, return -1
    // The calculated value is always larger than p.x or p.y
    // Must be in the middle 3/5 of the board
    int minWidth = avgSquareWidth / 5;
    int minHeight = avgSquareHeight / 5;
    int xDist = edgeX[0] + (pos.x+1)*avgSquareWidth - p.x;
    int yDist = edgeY[0] + (pos.y+1)*avgSquareHeight - p.y;
    if(xDist < minWidth || xDist > minWidth * 4) {
        // Piece is close to a border
        pos.x = -1;
        pos.y = -1;
    }
    /*if(yDist < minHeight || yDist > minHeight) {
        pos.x = -1;
        pos.y = -1;
    }*/
    std::cout << pos.x << ", " << pos.y << "\n";
    return pos;
}
