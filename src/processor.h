#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <opencv2/opencv.hpp>
#include <string>

// Function to process an image
void processImage(
    const std::string& rawImagePath, const std::string& fileName, const std::string& file, const std::string& modImageFilePath,
    const std::string& histDir, const std::string& mode, const std::string transformType, const int L, const bool verbose,
    const double inputScale = 0.2, const double clipLimit = 40, const cv::Size& tileGridSize = cv::Size(8, 8));

// Function to process a video
void processVideo(
    const std::string& rawVideoPath, const std::string& fileName, const std::string& modVideoFilePath, 
    const std::string& mode, const std::string& transformType, const int L, const bool verbose,
    const double inputScale = 0.2, const double clipLimit = 40, const cv::Size& tileGridSize = cv::Size(8, 8));

#endif