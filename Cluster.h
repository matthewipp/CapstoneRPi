/**
 * @file Cluster.h
 * @author EMNEM
 * @brief Defines Cluster class and Point struct used for piece recognition
 * @version 0.1
 * @date 2022-10-19
 */

#ifndef CLUSTER_H
#define CLUSTER_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

// Max distance in pixels squared
#define CLUSTER_MIN_POINTS 50
#define CLUSTER_KING_MIN_POINTS 50
#define CLUSTER_MAX_DISTANCE_SQUARE 200

enum PointType {RED, BLUE, YELLOW};

struct Point {
    int x;
    int y;
    enum PointType type;
};

class Cluster {
    public:
        // Initialize object
        Cluster();
        // Contains info about group of pixels and how to add new ones
        // point is struct of int x, int y, int type), where 0 = blue, 1 = red, 2 = yellow
        void addPoint(Point& p);
        // Checks if point is within range
        // returns True or False depending on whether close point found
        bool checkRange(Point& p1);
        // Finalizes cluster and returns true or false depending
        // on whether cluster is likely a piece
        bool finalize();
        // Variables
        int red;
        int blue;
        int yellow;
        bool isBlue;
        bool isKing;
        bool isValid;
        std::vector<Point> points;
        int xSum;
        int ySum;
        int x;
        int y;
};

#endif
