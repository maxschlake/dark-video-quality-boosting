#ifndef UTILS_H
#define UTILS_H

#include <opencv2/opencv.hpp>
#include <map>

// Function to fit an image to a window
cv::Mat fitImageToWindow(const cv::Mat& image, int windowMaxWidth, int windowMaxHeight);

// Function to apply color channel stretching
void stretchColorChannels(const cv::Mat& image, int minLim, int MaxLim);

// Function to apply the logarithmic transformation
void transformLogarithmic(const cv::Mat& image, double inputScale, int maxLim);

// Function to apply histogram equalization, either locally (CLAHE) or globally
void transformHistEqual(const cv::Mat& image, double clipLimit = 40, cv::Size tileGridSize = cv::Size(8, 8), const std::string& equalType = "local");

// Function to apply a BGR to HSI transformation
cv::Mat transformBGRToHSI(cv::Mat& image, double maxLim, const std::string& scaleType = "normalized");

// Function to compute a histogram for a certain channel
std::map<double, int> computeChannelHist(const cv::Mat& image, int channelIndex);

// Function to compute the clipping limit for a given channel histogram
double computeClippingLimit(const std::map<double, int>& channelHist, int L);

// Function to compute a clipped histogram for a certain channel, based on a clipping limit
std::map<double, double> computeClippedChannelHist(const std::map<double, int>& channelHist, double clippingLimit, int& M);

// Function to compute the PDF for a certain channel, based on a clipped histogram
std::map<double, double> computePDF(const std::map<double, double>& clippedChannelHist, int M);

// Function to compute the CDF for a certain channel, based on a PDF
std::map<double, double> computeCDF(const std::map<double, double>& PDF);

void plotHistogram(const std::map<double, int>& histMap);
void plotHistogram2(const std::map<double, double>& clippedHistMap);

#endif