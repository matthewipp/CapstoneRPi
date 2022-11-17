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

const char ImageState::STARTING_BOARD[8][8] = {{ 0 ,'r', 0 , 0 , 0 ,'b', 0 ,'b'},
                                               {'r', 0 ,'r', 0 , 0 , 0 ,'b', 0 },
                                               { 0 ,'r', 0 , 0 , 0 ,'b', 0 ,'b'},
                                               {'r', 0 ,'r', 0 , 0 , 0 ,'b', 0 },
                                               { 0 ,'r', 0 , 0 , 0 ,'b', 0 ,'b'},
                                               {'r', 0 ,'r', 0 , 0 , 0 ,'b', 0 },
                                               { 0 ,'r', 0 , 0 , 0 ,'b', 0 ,'b'},
                                               {'r', 0 ,'r', 0 , 0 , 0 ,'b', 0 }};

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

bool ImageState::generateBoardstate(cv::Mat& img, bool checkLegalMove) {
    majorFault = false;
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
    if(!majorFault && (valid && wasLegalMove || !checkLegalMove)) {
        std::memcpy(lastValidBoardState, boardState, sizeof(boardState));
        return true;
    }
    else {
        return false;
    }
}

bool ImageState::generateBoardState(std::vector<Cluster>& redClusters, 
                                    std::vector<Cluster>& blueClusters) {
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
    // CLear boardstate
    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            boardState[i][j] = 0;
        }
    }
    for(CheckersPiece& p : redPiecesOnBoard) {
        cv::Point2i coord = getBoardPos(p);
        if(coord.x == -1) {
            // Invalid coordinate
            success = false;
        }
        else if(boardState[coord.x][coord.y] == 0) {
            if(p.isKing) {
                boardState[coord.x][coord.y] = 'R';
            } 
            else {
                boardState[coord.x][coord.y] = 'r';
            }
            //proposedBoardStatePointer[coord.x][coord.y] = &p;
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
        else if(boardState[coord.x][coord.y] == 0) {
            if(p.isKing) {
                boardState[coord.x][coord.y] = 'B';
            } 
            else {
                boardState[coord.x][coord.y] = 'b';
            }
            //proposedBoardStatePointer[coord.x][coord.y] = &p;
        } 
        else {
            // Two pieces on same spot
            success = false;
        }
    }
    //std::memcpy(proposedBoardState, boardState, sizeof(boardState));
    //std::memcpy(proposedBoardStatePointer, boardStatePointer, sizeof(boardStatePointer));
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
    int lr1 = (int)(leftSum / cornerHeight);
    int lr2 = (int)(rightSum / cornerHeight);
    int tb1 = (int)(topSum / cornerWidth);
    int tb2 = (int)(botSum / cornerWidth);
    if(lr1 < lr2) {
        edgeY[0] = lr1 - avgSquareWidth;
        edgeY[1] = lr2 + avgSquareWidth;
    }
    else {
        edgeY[0] = lr2 - avgSquareWidth;
        edgeY[1] = lr1 + avgSquareWidth;
    }
    if(tb1 < tb2) {
        edgeX[0] = tb1 - avgSquareHeight;
        edgeX[1] = tb2 + avgSquareHeight;
    }
    else {
        edgeX[0] = tb2 - avgSquareHeight;
        edgeX[1] = tb1 + avgSquareHeight;
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
    int minWidth = avgSquareHeight / 5;
    int minHeight = avgSquareWidth / 5;
    int xDist = edgeX[0] + (pos.x+1)*avgSquareHeight - p.x;
    int yDist = edgeY[0] + (pos.y+1)*avgSquareWidth - p.y;
    if(xDist < minWidth || xDist > minWidth * 4) {
        // Piece is close to a border
        pos.x = -1;
        pos.y = -1;
    }
    if(yDist < minHeight || yDist > minHeight * 4) {
        pos.x = -1;
        pos.y = -1;
    }
    return pos;
}

void ImageState::createMoveList(std::list<ImageMove>& moveList) {
    createMoveList(moveList, STARTING_BOARD);
}

void ImageState::createMoveList(std::list<ImageMove>& moveList, const char desiredBoard[8][8]) {
    // Setup
    majorFault = false;
    moveList.clear();
    std::vector<IncorrectSquare> shouldBeEmpty;
    std::vector<IncorrectSquare> shouldBeFilled;
    std::vector<CheckersPiece*> matchedPieces;
    // Classify incorrect squares
    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            if(boardState[i][j] != desiredBoard[i][j]) {
                if(desiredBoard[i][j] == 0) {
                    // Should be empty
                    IncorrectSquare square;
                    square.x = i;
                    square.y = j;
                    square.occupiedPiece = boardStatePointer[i][j];
                    square.matchedPiece = nullptr;
                    shouldBeEmpty.push_back(square);
                }
                else {
                    // Should be filled with new piece
                    IncorrectSquare square;
                    square.x = i;
                    square.y = j;
                    square.occupiedPiece = boardStatePointer[i][j];
                    square.matchedPiece = nullptr;
                    shouldBeFilled.push_back(square);
                }
            }
        }
    }
    // Match incorrect squares to pieces
    for(IncorrectSquare& s : shouldBeFilled) {
        bool shouldBeBlue = false;
        bool shouldBeKing = false;
        if(desiredBoard[s.x][s.y] == 'b' || desiredBoard[s.x][s.y] == 'B') {
            shouldBeBlue = true;
        }
        if(desiredBoard[s.x][s.y] == 'R' || desiredBoard[s.x][s.y] == 'B') {
            shouldBeKing = true;
        }
        for(IncorrectSquare& sTarget : shouldBeEmpty) {
            if(sTarget.occupiedPiece->isBlue == shouldBeBlue && 
                        sTarget.occupiedPiece->isKing == shouldBeKing) {
                // Piece can be matched
                s.matchedPiece = sTarget.occupiedPiece;
                sTarget.matchedPiece = sTarget.occupiedPiece;
                if(s.occupiedPiece != nullptr) {
                    // If target square has piece already, then remove it first
                    ImageMove premove;
                    bool foundSpot = findEmptySpotOffBoard(premove, sTarget);
                    if(!foundSpot) {
                        majorFault = true;
                        return;
                    }
                    moveList.push_back(premove);
                }
                ImageMove move;
                move.startX = s.occupiedPiece->imageX;
                move.startY = s.occupiedPiece->imageY;
                getSquareCoords(move.endX, move.endY, sTarget.x, sTarget.y);
                moveList.push_back(move);
                break;
            }
        }
    }
    // Remove incorrect unmatched pieces
    for(IncorrectSquare& s : shouldBeEmpty) {
        if(s.matchedPiece == nullptr) {
            // piece has not been matched
            ImageMove move;
            bool foundSpot = findEmptySpotOffBoard(move, s);
            if(!foundSpot) {
                majorFault = true;
                return;
            }
            moveList.push_back(move);
        }
    }
    // Add pieces to board from off board
    for(IncorrectSquare& s : shouldBeFilled) {
        if(s.matchedPiece == nullptr) {
            if(s.occupiedPiece != nullptr) {
                ImageMove premove;
                    bool foundSpot = findEmptySpotOffBoard(premove, s);
                    if(!foundSpot) {
                        majorFault = true;
                        return;
                    }
                    moveList.push_back(premove);
            }
            ImageMove move;
            bool foundPiece = findPieceFromOffBoard(move, s, desiredBoard[s.x][s.y]);
            if(!foundPiece) {
                majorFault = true;
                return;
            }
            moveList.push_back(move);
        }
    }
    printBoardState(desiredBoard);
}

bool ImageState::findEmptySpotOffBoard(ImageMove& move, IncorrectSquare& s) {
    int minSquareDist = (avgSquareWidth * avgSquareWidth * 49) / 64;
    bool found = false;
    // Check top
    for(int i = 0; i < TOP_ROWS && !found; i++) {
        for(int j = 0; j < 8 && !found; j++) {
            bool empty = true;
            int sX = edgeX[0] + avgSquareWidth * j + (avgSquareWidth/2);
            int sY = edgeY[0] - avgSquareHeight * i - (avgSquareHeight/2);
            for(CheckersPiece rp : redPiecesOffBoard) {
                if((rp.imageX - sX)*(rp.imageX - sX)+(rp.imageY - sY)*(rp.imageY - sY) < minSquareDist) {
                    // Piece occupied
                    empty = false;
                    break;
                }
            }
            if(empty) {
                for(CheckersPiece bp : bluePiecesOffBoard) {
                    if((bp.imageX - sX)*(bp.imageX - sX)+(bp.imageY - sY)*(bp.imageY - sY) < minSquareDist) {
                        // Piece occupied
                        empty = false;
                        break;
                    }
                }
            }
            if(empty) {
                found = true;
                s.matchedPiece = s.occupiedPiece;
                getSquareCoords(move.startX, move.startY, s.x, s.y);
                move.endX = sX;
                move.endY = sY;
            }
        }
    }
    // Check right
    for(int i = 0; i < RIGHT_COLS && !found; i++) {
        for(int j = 0; j < 8 && !found; j++) {
            bool empty = true;
            int sX = edgeX[1] + avgSquareWidth * i + (avgSquareWidth/2);
            int sY = edgeY[0] + avgSquareHeight * j - (avgSquareHeight/2);
            for(CheckersPiece rp : redPiecesOffBoard) {
                if((rp.imageX - sX)*(rp.imageX - sX)+(rp.imageY - sY)*(rp.imageY - sY) < minSquareDist) {
                    // Piece occupied
                    empty = false;
                    break;
                }
            }
            if(empty) {
                for(CheckersPiece bp : bluePiecesOffBoard) {
                    if((bp.imageX - sX)*(bp.imageX - sX)+(bp.imageY - sY)*(bp.imageY - sY) < minSquareDist) {
                        // Piece occupied
                        empty = false;
                        break;
                    }
                }
            }
            if(empty) {
                found = true;
                s.matchedPiece = s.occupiedPiece;
                getSquareCoords(move.startX, move.startY, s.x, s.y);
                move.endX = sX;
                move.endY = sY;
            }
        }
    }
    // Check Bot
    for(int i = 0; i < TOP_ROWS && !found; i++) {
        for(int j = 0; j < 8 && !found; j++) {
            bool empty = true;
            int sX = edgeX[0] + avgSquareWidth * j + (avgSquareWidth/2);
            int sY = edgeY[1] + avgSquareHeight * i + (avgSquareHeight/2);
            for(CheckersPiece rp : redPiecesOffBoard) {
                if((rp.imageX - sX)*(rp.imageX - sX)+(rp.imageY - sY)*(rp.imageY - sY) < minSquareDist) {
                    // Piece occupied
                    empty = false;
                    break;
                }
            }
            if(empty) {
                for(CheckersPiece bp : bluePiecesOffBoard) {
                    if((bp.imageX - sX)*(bp.imageX - sX)+(bp.imageY - sY)*(bp.imageY - sY) < minSquareDist) {
                        // Piece occupied
                        empty = false;
                        break;
                    }
                }
            }
            if(empty) {
                found = true;
                s.matchedPiece = s.occupiedPiece;
                getSquareCoords(move.startX, move.startY, s.x, s.y);
                move.endX = sX;
                move.endY = sY;
            }
        }
    }
    // Check left
    for(int i = 0; i < RIGHT_COLS && !found; i++) {
        for(int j = 0; j < 8 && !found; j++) {
            bool empty = true;
            int sX = edgeX[1] - avgSquareWidth * i - (avgSquareWidth/2);
            int sY = edgeY[0] + avgSquareHeight * j + (avgSquareHeight/2);
            for(CheckersPiece rp : redPiecesOffBoard) {
                if((rp.imageX - sX)*(rp.imageX - sX)+(rp.imageY - sY)*(rp.imageY - sY) < minSquareDist) {
                    // Piece occupied
                    empty = false;
                    break;
                }
            }
            if(empty) {
                for(CheckersPiece bp : bluePiecesOffBoard) {
                    if((bp.imageX - sX)*(bp.imageX - sX)+(bp.imageY - sY)*(bp.imageY - sY) < minSquareDist) {
                        // Piece occupied
                        empty = false;
                        break;
                    }
                }
            }
            if(empty) {
                found = true;
                s.matchedPiece = s.occupiedPiece;
                getSquareCoords(move.startX, move.startY, s.x, s.y);
                move.endX = sX;
                move.endY = sY;
            }
        }
    }
    return found;
}

bool ImageState::findPieceFromOffBoard(ImageMove& move, IncorrectSquare& s, char piece) {
    bool isKing = false;
    bool found = false;
    if(piece == 'B' || piece == 'R') {
        isKing = true;
    }
    if(piece == 'R' || piece == 'r') {
        for(CheckersPiece& cp : redPiecesOffBoard) {
            if(cp.isKing == isKing) {
                found = true;
                s.matchedPiece = &cp;
                move.startX = cp.imageX;
                move.startY = cp.imageY;
                getSquareCoords(move.endX, move.endY, s.x, s.y);
            }
        }
    }
    if(piece == 'B' || piece == 'b') {
        for(CheckersPiece& cp : bluePiecesOffBoard) {
            if(cp.isKing == isKing) {
                found = true;
                s.matchedPiece = &cp;
                move.startX = cp.imageX;
                move.startY = cp.imageY;
                getSquareCoords(move.endX, move.endY, s.x, s.y);
            }
        }
    }
    return found;
}

void ImageState::getSquareCoords(int& imageX, int& imageY, int squareX, int squareY) {
    imageX = squareX * avgSquareWidth + edgeX[0] + (avgSquareWidth/2);
    imageY = squareY * avgSquareHeight + edgeY[0] + (avgSquareHeight/2);
}

void printBoardState(const char boardState[8][8]) {
    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            char print = boardState[j][i];
            if(print == 0) {
                print = '.';
            }
            std::cout << " " << print;
        }
        std::cout << "\n";
    }
}
