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
std::map<double, int> computeChannelHist(const cv::Mat& image, int channelIndex, int L, double& cMax);

// Function to compute the clipping limit for a given channel histogram
double computeClippingLimit(const std::map<double, int>& channelHist, int L);

// Function to compute a clipped histogram for a certain channel, based on a clipping limit
std::map<double, double> computeClippedChannelHist(const std::map<double, int>& channelHist, double clippingLimit, int& M);

// Function to compute the PDF for a certain channel, based on a clipped histogram
std::map<double, double> computePDF(const std::map<double, double>& clippedChannelHist, int M, double& pmax, double& pmin);

// Function to compute the CDF for a certain channel, based on a PDF
std::map<double, double> computeCDF(const std::map<double, double>& PDF);

// Function to compute the weighted histogram distribution (WHD) function and WHDFSum, based on PDF, CDF, pmax, pmin and cMax
std::map<double, double> computeWHDF(const std::map<double, double>& PDF, const std::map<double, double>& CDF, double& WHDFSum, double pmax, double pmin, double cMax);

// Function to compute Gammabased on the weighted histogram distribution (WHD) function, WHDFSum and cMax
std::map<double, double> computeGamma(const std::map<double, double>& WHDF, double WHDFSum, double cMax);

// Function to transform a channel, based on the gamma function
cv::Mat transformChannel(cv::Mat image, int channelIndex, std::map<double, double> gamma, double cMax);

// Function to apply an HSI to BGR transformation
cv::Mat transformHSIToBGR(cv::Mat& image, double maxLim, const std::string& inputScaleType = "normalized");

// Template definition for the plotHistogram function
template <typename T>
void plotHistogram(const std::map<double, T>& histMap) {
    int histSize = histMap.size();   // Number of unique intensity values
    int histHeight = 600;            // Height of the histogram image
    int histWidth = 800;             // Width of the histogram image
    int binWidth = cvRound(static_cast<double>(histWidth) / histSize);

    // Create a blank image for the histogram plot with padding for labels and ticks
    cv::Mat histImage(histHeight + 60, histWidth + 60, CV_8UC3, cv::Scalar(255, 255, 255));

    // Find the max count to normalize histogram heights and increase for buffer
    T maxCount = 0;
    for (const auto& pair : histMap) {
        if (pair.second > maxCount) {
            maxCount = pair.second;
        }
    }
    maxCount = static_cast<T>(std::ceil(maxCount * 1.1 / 500) * 500);  // Round up to nearest multiple of 500

    // Draw histogram bins in dark blue
    int i = 0;
    for (const auto& pair : histMap) {
        int binHeight = cvRound(static_cast<double>(pair.second) / maxCount * histHeight);
        cv::rectangle(histImage,
                      cv::Point(i * binWidth + 40, histHeight),
                      cv::Point((i + 1) * binWidth + 40, histHeight - binHeight),
                      cv::Scalar(139, 0, 0), cv::FILLED);  // Dark blue color for bars
        i++;
    }

    // Draw X-axis ticks and labels
    int tickInterval = 50;  // Interval for x-axis ticks
    for (int x = 0; x <= histWidth; x += tickInterval * histWidth / 256) {
        int intensityValue = (x * 256 / histWidth);  // Adjusted to map correctly to intensity values
        if (intensityValue % 50 == 0) {  // Draw labels at multiples of 50
            cv::line(histImage, cv::Point(x + 40, histHeight), cv::Point(x + 40, histHeight + 5), cv::Scalar(0, 0, 0), 1);
            cv::putText(histImage, std::to_string(intensityValue), cv::Point(x + 30, histHeight + 20), cv::FONT_HERSHEY_DUPLEX, 0.4, cv::Scalar(0, 0, 0), 1);
        }
    }

    // Add explicit tick marks at 100 and 200
    for (int intensityValue : {100, 200}) {
        int x = intensityValue * histWidth / 256;  // Mapping intensity value to x-coordinate
        cv::line(histImage, cv::Point(x + 40, histHeight), cv::Point(x + 40, histHeight + 5), cv::Scalar(0, 0, 0), 1);
        cv::putText(histImage, std::to_string(intensityValue), cv::Point(x + 30, histHeight + 20), cv::FONT_HERSHEY_DUPLEX, 0.4, cv::Scalar(0, 0, 0), 1);
    }

    // Calculate Y-axis ticks
    int tickCount = 4;  // We want exactly 4 ticks
    int yTickInterval = static_cast<int>(std::ceil(maxCount / (tickCount - 1) / 1000.0)) * 1000; // Ensure the interval is a multiple of 1000

    // Set the tick values based on calculated interval
    std::vector<int> yTicks(tickCount);
    for (int j = 0; j < tickCount; j++) {
        yTicks[j] = j * yTickInterval;  // Compute each tick value
    }

    // Draw Y-axis ticks and labels
    for (int frequencyValue : yTicks) {
        // Calculate the Y position for each tick
        int y = histHeight - (frequencyValue * histHeight / (maxCount + yTickInterval)); // Map frequency to Y coordinate
        if (y >= 0 && y <= histHeight) { // Ensure y position is within bounds
            cv::line(histImage, cv::Point(35, y), cv::Point(40, y), cv::Scalar(0, 0, 0), 1);
            cv::putText(histImage, std::to_string(frequencyValue), cv::Point(5, y + 5), cv::FONT_HERSHEY_DUPLEX, 0.4, cv::Scalar(0, 0, 0), 1);
        }
    }

    // Draw axis rectangle for histogram
    cv::rectangle(histImage, cv::Point(40, 0), cv::Point(histWidth + 40, histHeight), cv::Scalar(0, 0, 0), 1);

    // Add X-axis label
    cv::putText(histImage, "Intensity", cv::Point(histWidth / 2 + 20, histHeight + 50), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(0, 0, 0), 1);

    // Display histogram
    cv::imshow("Histogram", histImage);
    cv::waitKey(0);
}

#endif