/**
 * @file PieceRecognition.h
 * @author EMNEM
 * @brief Contains helper functions for using piece recognition
 * @version 0.1
 * @date 2022-10-19
 */

#ifndef PIECE_RECOGNITION_H
#define PIECE_RECOGNITION_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include "ImageState.h"
#include "Cluster.h"

#define KERNEL_SIZE 4
#define RED_FILTER_AVERAGE_CUTOFF 60
#define BLUE_FILTER_AVERAGE_CUTOFF 60
#define YELLOW_FILTER_AVERAGE_CUTOFF 120

// Returns list of points for both red and blue pieces
void getPointsInImage(cv::Mat& img, std::vector<std::vector<Point>>& pointsList);
// Turns list of Points into list of clusters
void clusterize(std::vector<Point>& pList, bool isBlue, std::vector<Cluster>& finalClustersint, int minX, int minY, int maxX, int maxY);
// Tale picture
bool takePicture(cv::Mat& img);

#endif