#ifndef UTILS_H
#define UTILS_H

#include <opencv2/opencv.hpp>

// Function to fit an image to a window
cv::Mat fitImageToWindow(const cv::Mat& image, int windowMaxWidth, int windowMaxHeight);

// Function to apply color channel stretching
void stretchColorChannels(const cv::Mat& image, int minLim, int MaxLim);

// Function to apply the logarithmic transformation
void transformLogarithmic(const cv::Mat& image, double inputScale, int maxLim);

#endif