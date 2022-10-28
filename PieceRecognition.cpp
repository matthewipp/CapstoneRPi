/**
 * @file PieceRecognition.cpp
 * @author EMNEM
 * @brief Implementation of piece recogntion helper functions
 * @version 0.1
 * @date 2022-10-19
 */

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include "PieceRecognition.h"
#include "ImageState.h"
#include "Cluster.h"

void readBoard(std::string state) {
    return;
}

void getPointsInImage(cv::Mat& img, std::vector<std::vector<Point>>& pointsList) {
    // Split RGB channels
    cv::Mat bgr[3];
    cv::split(img, bgr);
    cv::Mat blueChannel, greenChannel, redChannel;
    bgr[0].convertTo(blueChannel, CV_32SC1);
    bgr[1].convertTo(greenChannel, CV_32SC1);
    bgr[2].convertTo(redChannel, CV_32SC1);
    // Filter
    cv::Mat blueFilter = 2*blueChannel - redChannel - greenChannel;
    cv::Mat redFilter = 2*redChannel - blueChannel - greenChannel;
    cv::Mat yellowFilter = (redChannel + greenChannel) - 2*blueChannel;
    // Find points
    int kernelSize = KERNEL_SIZE;
    int kernelArea = kernelSize * kernelSize;
    double redFilterCutoff = RED_FILTER_AVERAGE_CUTOFF * kernelArea;
    double blueFilterCutoff = BLUE_FILTER_AVERAGE_CUTOFF * kernelArea;
    double yellowFilterCutoff = YELLOW_FILTER_AVERAGE_CUTOFF * kernelArea;
    std::vector<Point> bluePoints;
    std::vector<Point> redPoints;
    for(int i = 0; i < 1088; i += kernelSize) {
        for(int j = 0; j < 1920; j += kernelSize) {
            cv::Mat blueSquare = blueFilter(cv::Rect(j, i, kernelSize, kernelSize));
            cv::Mat redSquare = redFilter(cv::Rect(j, i, kernelSize, kernelSize));
            cv::Mat yellowSquare = yellowFilter(cv::Rect(j, i, kernelSize, kernelSize));
            // Check for yellow
            if(cv::sum(yellowSquare)[0] > yellowFilterCutoff) {
                Point p;
                p.x = i + kernelSize/2;
                p.y = j + kernelSize/2;
                p.type = YELLOW;
                bluePoints.push_back(p);
                redPoints.push_back(p);
            }
            // Check for blue
            else if(cv::sum(blueSquare)[0] > blueFilterCutoff) {
                Point p;
                p.x = i + kernelSize/2;
                p.y = j + kernelSize/2;
                p.type = BLUE;
                bluePoints.push_back(p);
            }
            // Check for red
            else if(cv::sum(redSquare)[0] > redFilterCutoff) {
                Point p;
                p.x = i + kernelSize/2;
                p.y = j + kernelSize/2;
                p.type = RED;
                redPoints.push_back(p);
            }
        }
    }
    // Add to vector and return
    pointsList.push_back(redPoints);
    pointsList.push_back(bluePoints);
    return;
}

void clusterize(std::vector<Point>& pList, bool isBlue, std::vector<Cluster>& finalClusters) {
    std::vector<Cluster> clusters;
    for(Point& p : pList) {
        // Go through all clusters and stop after finding one
        bool found = false;
        for(int i = (int)clusters.size()-1; i >= 0; i--) {
            Cluster& c = clusters[i];
            if(c.checkRange(p)) {
                c.addPoint(p);
                found = true;
                break;
            }
        }
        if(!found) {
            Cluster newCluster;
            newCluster.addPoint(p);
            clusters.push_back(newCluster);
        }
    }
    // Finish cluster calculation
    for(Cluster& c : clusters) {
        c.finalize();
        if(c.isValid && c.isBlue == isBlue) {
            finalClusters.push_back(c);
        }
    }
    return;
}
