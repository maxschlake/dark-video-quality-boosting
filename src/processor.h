#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <opencv2/opencv.hpp>
#include <string>

// Function to process an image
void processImage(
    const std::string& rawImagePath, const std::string& modImagePath, const std::string& fileName, const std::string& mode,
    const std::string& filePath, const std::string& histPath, const std::string transformType, int L, bool verbose, double inputScale,
    double clipLimit = 40, const cv::Size tileGridSize = cv::Size(8, 8));

// Function to process a video
void processVideo(
    const std::string& rawVideoPath, const std::string& modVideoDir, const std::string& fileName, const std::string& mode, 
    const std::string& transformType, int L, bool verbose, double inputScale,
    double clipLimit = 40, const cv::Size tileGridSize = cv::Size(8, 8));

#endif