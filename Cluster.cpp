/**
 * @file Cluster.cpp
 * @author EMNEM
 * @brief Implementation of CLuster class
 * @version 0.1
 * @date 2022-10-19
 */

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include "Cluster.h"

Cluster::Cluster() {
    red = 0;
    blue = 0;
    yellow = 0;
    isBlue = false;
    isKing = false;
    isValid = false;
    xSum = 0;
    ySum = 0;
    x = 0;
    y = 0;
}

void Cluster::addPoint(Point& p) {
    // Add point to list and update sums
    points.push_back(p);
    xSum += p.x;
    ySum += p.y;
    // Check color and add to appropriate counter
    if(p.type == RED) {
        red += 1;
    }
    else if(p.type == BLUE) {
        blue += 1;
    }
    else {
        yellow += 1;
    }
}

bool Cluster::checkRange(Point& p1) {
    bool found = false;
    // Check for close points until one is found
    // BREAKS WHEN POINT IS FOUND
    for(Point& p2 : points) {
        // Check if points are same color or one is yellow
        if(p2.type == YELLOW || p1.type == YELLOW || p2.type == p1.type) {
            // Check distance
            int xDiff = p2.x - p1.x;
            int yDiff = p2.y - p1.y;
            if(xDiff*xDiff + yDiff*yDiff < CLUSTER_MAX_DISTANCE_SQUARE) {
                // Point is within range
                addPoint(p1);
                found = true;
                break;
            } 
        }
    }
    // Return whether close point was found
    return found;
}

bool Cluster::finalize() {
    // Check total number of points
    int totalPoints = red + blue + yellow;
    if(totalPoints < CLUSTER_MIN_POINTS) {
        // Not enough points to be piece
        return false;
    }
    // Check color
    if(red > blue) {
        isBlue = false;
        if(red < CLUSTER_MIN_POINTS) {
            // Not enough red points to be piece
            return false;
        }
    }
    else {
        isBlue = true;
        if(blue < CLUSTER_MIN_POINTS) {
            // Not enough blue points to be piece
            return false;
        }
    }
    // Check for king
    if(yellow >= CLUSTER_KING_MIN_POINTS) {
        isKing = true;
    }
    else {
        isKing = false;
    }
    // Calculate center
    x = xSum / totalPoints;
    y = ySum / totalPoints;
    isValid = true;
    return true;
}
