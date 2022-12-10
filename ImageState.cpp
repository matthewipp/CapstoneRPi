/**
 * @file ImageState.cpp
 * @author EMNEM
 * @brief Implementation of ImageState class for piece recognition
 * @version 0.1
 * @date 2022-10-19
 */

#include "ImageState.h"
#include <algorithm>
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include "Cluster.h"
#include "PieceRecognition.h"
#include "bot.h"

const char ImageState::STARTING_BOARD[8][8] = {{ 0 ,'r', 0 , 0 , 0 ,'b', 0 ,'b'},
                                               {'r', 0 ,'r', 0 , 0 , 0 ,'b', 0 },
                                               { 0 ,'r', 0 , 0 , 0 ,'b', 0 ,'b'},
                                               {'r', 0 ,'r', 0 , 0 , 0 ,'b', 0 },
                                               { 0 ,'r', 0 , 0 , 0 ,'b', 0 ,'b'},
                                               {'r', 0 ,'r', 0 , 0 , 0 ,'b', 0 },
                                               { 0 ,'r', 0 , 0 , 0 ,'b', 0 ,'b'},
                                               {'r', 0 ,'r', 0 , 0 , 0 ,'b', 0 }};

bool operator==(CheckersPiece const& lhs, CheckersPiece const& rhs) {
    bool coordsMatch = (lhs.imageX == rhs.imageX) && (lhs.imageY == rhs.imageY);
    bool typeMatch = (lhs.isBlue == rhs.isBlue) && (lhs.isKing == rhs.isKing);
    return coordsMatch && typeMatch;
}

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
    if(bluePoints.size() + redPoints.size() > 10000) {
        majorFault = true;
        return false;
    }
    std::vector<Cluster> blueClusters;
    int minX = edgeX[0] - (113 * avgSquareWidth) / 100;
    int minY = edgeY[0] - (120 * avgSquareHeight) / 100;
    int maxX = edgeX[1] + (185 * avgSquareWidth) / 100;
    int maxY = edgeY[1] + (185 * avgSquareHeight) / 100;
    clusterize(bluePoints, true, blueClusters, minX, minY, maxX, maxY);
    std::vector<Cluster> redClusters;
    clusterize(redPoints, false, redClusters, minX, minY, maxX, maxY);
    bool valid = generateBoardState(redClusters, blueClusters);
    isValidState = valid;
    // check if the move was legal here
    Bot jimmyJr;
    jimmyJr.bot_color = true;
    bool wasLegalMove = jimmyJr.comp_boards(lastValidBoardState, boardState);
    std::cout << "Jimmy Jr.'s Verdict: " << (wasLegalMove ? "Legal" : "Illegal") << std::endl;
    //wasLegalMove = true;
    printBoardState(boardState);
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
    redErrorPiecesOnBoard.clear();
    blueErrorPiecesOnBoard.clear();
    // Check positions on board
    // CLear boardstate
    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            boardState[i][j] = 0;
            boardStatePointer[i][j] = nullptr;
        }
    }
    // Red Pieces
    for(Cluster& c : redClusters) {
        CheckersPiece cp;
        cp.isBlue = false;
        cp.isKing = c.isKing;
        cp.imageX = c.x;
        cp.imageY = c.y;
        cp.x = -1;
        cp.y = -1;
        cp.onBoard = edgeX[0] < cp.imageX && cp.imageX < edgeX[1] && edgeY[0] < cp.imageY && cp.imageY < edgeY[1];
        if(cp.onBoard)
        {
            cv::Point2i coord = getBoardPos(cp);
            cp.x = coord.x;
            cp.y = coord.y;
            if(coord.x == -1) {
                // Invalid coordinate
                success = false;
                redErrorPiecesOnBoard.push_back(cp);
            }
            else if(boardState[coord.x][coord.y] == 0) {
                if(cp.isKing) {
                    boardState[coord.x][coord.y] = 'R';
                } 
                else {
                    boardState[coord.x][coord.y] = 'r';
                }
                redPiecesOnBoard.push_back(cp);
                boardStatePointer[coord.x][coord.y] = &(redPiecesOnBoard.back());
            } 
            else {
                // Two pieces on same spot
                success = false;
            }
        }
        else{
            redPiecesOffBoard.push_back(cp);
        }
    }
    // Blue Pieces
    for(Cluster& c : blueClusters) {
        CheckersPiece cp;
        cp.isBlue = true;
        cp.isKing = c.isKing;
        cp.imageX = c.x;
        cp.imageY = c.y;
        cp.x = -1;
        cp.y = -1;
        cp.onBoard = edgeX[0] < cp.imageX && cp.imageX < edgeX[1] && edgeY[0] < cp.imageY && cp.imageY < edgeY[1];
        if(cp.onBoard)
        {
            cv::Point2i coord = getBoardPos(cp);
            cp.x = coord.x;
            cp.y = coord.y;
            if(coord.x == -1) {
                // Invalid coordinate
                success = false;
                blueErrorPiecesOnBoard.push_back(cp);
            }
            else if(boardState[coord.x][coord.y] == 0) {
                if(cp.isKing) {
                    boardState[coord.x][coord.y] = 'B';
                } 
                else {
                    boardState[coord.x][coord.y] = 'b';
                }
                bluePiecesOnBoard.push_back(cp);
                boardStatePointer[coord.x][coord.y] = &(bluePiecesOnBoard.back());
            } 
            else {
                // Two pieces on same spot
                success = false;
            }
        }
        else{
            bluePiecesOffBoard.push_back(cp);
        }
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
    // test orientation
    float testSumX = 0;
    float testSumY = 0;
    bool alongX = true;
    for(int i = 1; i < cornerWidth; i++) {
        testSumX += corners[i].x - corners[i-1].x;
        testSumY += corners[i].y - corners[i-1].y;
    }
    if(testSumX < 0) {
        testSumX *= -1;
    }
    if(testSumY < 0) {
        testSumY *= -1;
    }
    if(testSumX < testSumY) {
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
                heightSum += corners[i+1].y - corners[i].y;     
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
    avgSquareHeight = (int)avgHeight;
    avgSquareWidth = (int)avgWidth;
    if(avgSquareHeight < 0) {
        avgSquareHeight *= -1;
    }
    if(avgSquareWidth < 0) {
        avgSquareWidth *= -1;
    }
    // Extrapolate out to board edges, x and y from coordinates to board are switched
    int lr1 = (int)(leftSum / cornerWidth);
    int lr2 = (int)(rightSum / cornerWidth);
    int tb1 = (int)(topSum / cornerHeight);
    int tb2 = (int)(botSum / cornerHeight);
    if(lr1 < lr2) {
        edgeX[0] = lr1 - avgSquareWidth;
        edgeX[1] = lr2 + avgSquareWidth;
    }
    else {
        edgeX[0] = lr2 - avgSquareWidth;
        edgeX[1] = lr1 + avgSquareWidth;
    }
    if(tb1 < tb2) {
        edgeY[0] = tb1 - avgSquareHeight;
        edgeY[1] = tb2 + avgSquareHeight;
    }
    else {
        edgeY[0] = tb2 - avgSquareHeight;
        edgeY[1] = tb1 + avgSquareHeight;
    }
    std::cout << "EdgeX: " << edgeX[0] << " to " << edgeX[1] << "\n";
    std::cout << "Avg. Width: " << avgSquareWidth << "\n";
    std::cout << "EdgeY: " << edgeY[0] << " to " << edgeY[1] << "\n";
    std::cout << "Avg. Height: " << avgSquareHeight << "\n";
    return true;
}

cv::Point2i ImageState::getBoardPos(CheckersPiece& p) {
    // Keep moving right or down until you get past the point
    cv::Point2i pos(0, 0);
    while(p.imageX > edgeX[0] + (pos.x+1)*avgSquareWidth) {
        pos.x++;
    }
    while(p.imageY > edgeY[0] + (pos.y+1)*avgSquareHeight) {
        pos.y++;
    }
    // Check if pos is too far
    if(pos.x > 7 || pos.y > 7) {
        pos.x= -1;
        pos.y = -1;
        return pos;
    }
    // Check distance, if too far, return -1
    // The calculated value is always larger than p.x or p.y
    // Must be in the middle 3/5 of the board
    int minWidth = avgSquareWidth / 5;
    int minHeight = avgSquareHeight / 5;
    int xDist = edgeX[0] + (pos.x+1)*avgSquareWidth - p.imageX;
    int yDist = edgeY[0] + (pos.y+1)*avgSquareHeight - p.imageY;
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
    std::cout << "Classified Incorrect Squares\n";
    std::cout << "Should be empty: " << (int)shouldBeEmpty.size() << std::endl;
    std::cout << "Should be filled: " << (int)shouldBeFilled.size() << std::endl;
    std::cout << "Error pieces:" << (int)redErrorPiecesOnBoard.size() + (int)blueErrorPiecesOnBoard.size() << std::endl;
    // Try matching error pieces first
    for(CheckersPiece& cp : redErrorPiecesOnBoard) {
        ImageMove move;
        bool foundSpot = findEmptySpotOffBoard(move, cp);
        if(foundSpot) {
            moveList.push_back(move);
        }
        else {
            majorFault = true;
            return;
        }
    }
    // Match incorrect squares to pieces
    for(IncorrectSquare& s : shouldBeFilled) {
        bool filled = false;
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
                        sTarget.occupiedPiece->isKing == shouldBeKing && sTarget.matchedPiece == nullptr) {
                // Piece can be matched
                std::cout << "Piece matched\n";
                s.matchedPiece = sTarget.occupiedPiece;
                sTarget.matchedPiece = sTarget.occupiedPiece;
                if(s.occupiedPiece != nullptr) {
                    // If target square has piece already, then remove it first
                    ImageMove premove;
                    bool foundSpot = findEmptySpotOffBoard(premove, *(s.occupiedPiece));
                    if(!foundSpot) {
                        majorFault = true;
                        std::cout << "ERROR: Unable to find empty spot for replaced piece\n";
                        return;
                    }
                    if(sTarget.occupiedPiece->isBlue) {
                        removeFromBoard(bluePiecesOnBoard, *(s.occupiedPiece));
                    }
                    else {
                        removeFromBoard(redPiecesOnBoard, *(s.occupiedPiece));
                    }
                    std::cout << "Removed piece first\n";
                    moveList.push_back(premove);
                }
                ImageMove move;
                move.startX = sTarget.occupiedPiece->imageX;
                move.startY = sTarget.occupiedPiece->imageY;
                getSquareCoords(move.endX, move.endY, s.x, s.y);
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
            bool foundSpot = findEmptySpotOffBoard(move, *(s.occupiedPiece));
            if(!foundSpot) {
                majorFault = true;
                std::cout << "ERROR: Unable to remove an inaccurate piece\n";
                return;
            }
            if(s.occupiedPiece->isBlue) {
                removeFromBoard(bluePiecesOnBoard, *(s.occupiedPiece));
            }
            else {
                removeFromBoard(redPiecesOnBoard, *(s.occupiedPiece));
            }
            moveList.push_back(move);
        }
    }
    for(IncorrectSquare& s : shouldBeFilled) {
        if(s.matchedPiece == nullptr && s.occupiedPiece != nullptr) {
            ImageMove offMove;
            bool foundSpot = findEmptySpotOffBoard(offMove, *(s.occupiedPiece));
            if(!foundSpot) {
                majorFault = true;
                std::cout << "ERROR: Unable to find spot for replaced piece from off board\n";
                return;
            }
            if(s.occupiedPiece->isBlue) {
                removeFromBoard(bluePiecesOnBoard, *(s.occupiedPiece));
            }
            else {
                removeFromBoard(redPiecesOnBoard, *(s.occupiedPiece));
            }
            moveList.push_back(offMove);
        }
    }
    // Add pieces to board from off board
    for(IncorrectSquare& s : shouldBeFilled) {
        if(s.matchedPiece == nullptr) {
            ImageMove move;
            bool foundPiece = findPieceFromOffBoard(move, s, desiredBoard[s.x][s.y]);
            if(!foundPiece) {
                majorFault = true;
                std::cout << "ERROR: Unable to find piece from off board\n";
                //return;
            }
            moveList.push_back(move);
        }
    }
    for(ImageMove move : moveList) {
        int startX = move.startX;
        int startY = move.startY;
        int endX = move.endX;
        int endY = move.endY;
        imageToRealSpace(startX, startY, move.startX, move.startY);
        imageToRealSpace(endX, endY, move.endX, move.endY);
        std::cout << "Move from (";
        std::cout << startX << ", " << startY;
        std::cout << ") to (" << endX << ", " << endY<< ")\n";
    }
}

bool ImageState::findEmptySpotOffBoard(ImageMove& move, CheckersPiece& cp) {
    int minSquareDist = (avgSquareWidth * avgSquareWidth * 49) / 64;
    bool found = false;
    // Check top
    for(int i = 0; i < TOP_ROWS && !found; i++) {
        for(int j = 0; j < 8 && !found; j++) {
            bool empty = true;
            int sX = edgeX[0] + avgSquareWidth * j + (avgSquareWidth/2);
            int sY = edgeY[0] - avgSquareHeight * i - (avgSquareHeight/2);
            for(CheckersPiece& rp : redPiecesOffBoard) {
                if((rp.imageX - sX)*(rp.imageX - sX)+(rp.imageY - sY)*(rp.imageY - sY) < minSquareDist) {
                    // Piece occupied
                    empty = false;
                    break;
                }
            }
            if(empty) {
                for(CheckersPiece& bp : bluePiecesOffBoard) {
                    if((bp.imageX - sX)*(bp.imageX - sX)+(bp.imageY - sY)*(bp.imageY - sY) < minSquareDist) {
                        // Piece occupied
                        empty = false;
                        break;
                    }
                }
            }
            if(empty) {
                found = true;
                move.startX = cp.imageX;
                move.startY = cp.imageY;
                cp.imageX = sX;
                cp.imageY = sY;
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
            for(CheckersPiece& rp : redPiecesOffBoard) {
                if((rp.imageX - sX)*(rp.imageX - sX)+(rp.imageY - sY)*(rp.imageY - sY) < minSquareDist) {
                    // Piece occupied
                    empty = false;
                    break;
                }
            }
            if(empty) {
                for(CheckersPiece& bp : bluePiecesOffBoard) {
                    if((bp.imageX - sX)*(bp.imageX - sX)+(bp.imageY - sY)*(bp.imageY - sY) < minSquareDist) {
                        // Piece occupied
                        empty = false;
                        break;
                    }
                }
            }
            if(empty) {
                found = true;
                move.startX = cp.imageX;
                move.startY = cp.imageY;
                cp.imageX = sX;
                cp.imageY = sY;
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
            for(CheckersPiece& rp : redPiecesOffBoard) {
                if((rp.imageX - sX)*(rp.imageX - sX)+(rp.imageY - sY)*(rp.imageY - sY) < minSquareDist) {
                    // Piece occupied
                    empty = false;
                    break;
                }
            }
            if(empty) {
                for(CheckersPiece& bp : bluePiecesOffBoard) {
                    if((bp.imageX - sX)*(bp.imageX - sX)+(bp.imageY - sY)*(bp.imageY - sY) < minSquareDist) {
                        // Piece occupied
                        empty = false;
                        break;
                    }
                }
            }
            if(empty) {
                found = true;
                move.startX = cp.imageX;
                move.startY = cp.imageY;
                cp.imageX = sX;
                cp.imageY = sY;
                move.endX = sX;
                move.endY = sY;
            }
        }
    }
    // Check left
    for(int i = 0; i < RIGHT_COLS && !found; i++) {
        for(int j = 0; j < 8 && !found; j++) {
            bool empty = true;
            int sX = edgeX[0] - avgSquareWidth * i - (avgSquareWidth/2);
            int sY = edgeY[0] + avgSquareHeight * j + (avgSquareHeight/2);
            for(CheckersPiece& rp : redPiecesOffBoard) {
                if((rp.imageX - sX)*(rp.imageX - sX)+(rp.imageY - sY)*(rp.imageY - sY) < minSquareDist) {
                    // Piece occupied
                    empty = false;
                    break;
                }
            }
            if(empty) {
                for(CheckersPiece& bp : bluePiecesOffBoard) {
                    if((bp.imageX - sX)*(bp.imageX - sX)+(bp.imageY - sY)*(bp.imageY - sY) < minSquareDist) {
                        // Piece occupied
                        empty = false;
                        break;
                    }
                }
            }
            if(empty) {
                found = true;
                move.startX = cp.imageX;
                move.startY = cp.imageY;
                cp.imageX = sX;
                cp.imageY = sY;
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
        CheckersPiece chosen;
        for(CheckersPiece& cp : redPiecesOffBoard) {
            if(cp.isKing == isKing) {
                found = true;
                s.matchedPiece = &cp;
                move.startX = cp.imageX;
                move.startY = cp.imageY;
                getSquareCoords(move.endX, move.endY, s.x, s.y);
                chosen = cp;
                break;
            }
        }
        if(found) {
            addPieceToBoard(redPiecesOffBoard, chosen);
        }
    }
    if(piece == 'B' || piece == 'b') {
        CheckersPiece chosen;
        for(CheckersPiece& cp : bluePiecesOffBoard) {
            if(cp.isKing == isKing) {
                found = true;
                s.matchedPiece = &cp;
                move.startX = cp.imageX;
                move.startY = cp.imageY;
                getSquareCoords(move.endX, move.endY, s.x, s.y);
                chosen = cp;
                break;
            }
        }
        if(found) {
            addPieceToBoard(bluePiecesOffBoard, chosen);
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

bool ImageState::imageToRealSpace(int& realX, int& realY, int imgX, int imgY) {
    int relativeX = imgX - edgeX[0];
    int relativeY = imgY - edgeY[0];
    realX = (relativeX * REAL_IMG_SQUARE_SIZE) / avgSquareWidth + REAL_IMG_OFFSET;
    realY = (relativeY * REAL_IMG_SQUARE_SIZE) / avgSquareHeight + REAL_IMG_OFFSET;
    return true;
}

void ImageState::addPieceToBoard(std::list<CheckersPiece> originalList, CheckersPiece& piece) {
    if(piece.isBlue) {
        bluePiecesOnBoard.push_back(piece);
    }
    else {
        redPiecesOnBoard.push_back(piece);
    }
    auto it = std::find(originalList.begin(), originalList.end(), piece);
    if(it != originalList.end()) {
        originalList.erase(it);
    }
    else {
        std::cout << "Could not erase from original off board list\n";
    }
}

void ImageState::removeFromBoard(std::list<CheckersPiece> originalList, CheckersPiece piece) {
    char before;
    if(piece.isBlue) {
        before = 'b';
    }
    else {
        before = 'r';
    }
    if(piece.isKing) {
        before -= 32;
    }
    std::cout << "Entering remove from board: " << before<< "\n";
    if(piece.isBlue) {
        bluePiecesOffBoard.push_back(piece);
    }
    else {
        redPiecesOffBoard.push_back(piece);
    }
    auto it = std::find(originalList.begin(), originalList.end(), piece);
    if(it != originalList.end()) {
        originalList.erase(it);
    }
    else {
        std::cout << "Error: could not erase from original list\n";
    }

    char after;
    if(bluePiecesOffBoard.back().isBlue) {
        after = 'b';
    }
    else {
        after = 'r';
    }
    if(bluePiecesOffBoard.back().isKing) {
        after -= 32;
    }
    std::cout << "Exiting remove from board: " << after << "\n";
}
