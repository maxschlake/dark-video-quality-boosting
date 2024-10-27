#ifndef UTILS_H
#define UTILS_H

#include <opencv2/opencv.hpp>

// Function to fit an image to a window
cv::Mat fitImageToWindow(const cv::Mat& image, int windowMaxWidth, int windowMaxHeight);

// Function to apply color channel stretching
void stretchColorChannels(const cv::Mat& image, int minLim, int MaxLim);

// Function to apply the logarithmic transformation
void transformLogarithmic(const cv::Mat& image, double inputScale, int maxLim);

// Function to apply histogram equalization, either locally (CLAHE) or globally
void transformHistEqual(const cv::Mat& image, double clipLimit = 40, cv::Size tileGridSize = cv::Size(8, 8), const std::string& equalType = "local");

// Function to apply a BGR to HSI transformation
void transformBGRToHSI(cv::Mat& image, double maxLim); 

#endif