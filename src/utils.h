#ifndef UTILS_H
#define UTILS_H

#include <opencv2/opencv.hpp>
#include <map>

// Function to save images
bool saveImage(const cv::Mat& image, const std::string& path, bool verbose = false);

// Function to calculate the y-axis labels dynamically
void calculateYAxisLabels(int maxCount, int& yMax, int& yMid);

// Function to fit an image to a window
cv::Mat fitImageToWindow(const cv::Mat& image, int windowMaxWidth, int windowMaxHeight);

// Function to apply color channel stretching
void stretchColorChannels(const cv::Mat& image, int minL, int L);

// Function to apply the logarithmic transformation
void transformLogarithmic(const cv::Mat& image, double inputScale, int L);

// Function to apply histogram equalization, either locally (CLAHE) or globally
void transformHistEqual(const cv::Mat& image, double clipLimit = 40, const cv::Size tileGridSize = cv::Size(8, 8), const std::string& equalType = "local");

// Function to apply a BGR to HSI transformation
cv::Mat transformBGRToHSI(const cv::Mat& image, int L, const std::string& scaleType = "BGR");

// Function to compute a histogram for a certain channel
std::map<double, int> computeChannelHist(const cv::Mat& image, int channelIndex, int L, double& cMax, cv::Mat& targetChannel, std::vector<cv::Mat>& otherChannels, bool verbose = false);

// Function to compute the clipping limit for a given channel histogram
double computeClippingLimit(const std::map<double, int>& channelHist, int L, bool verbose = false);

// Function to compute a clipped histogram for a certain channel, based on a clipping limit
std::map<double, int> computeClippedChannelHist(const std::map<double, int>& channelHist, double clippingLimit, int& M, bool verbose = false);

// Function to compute the PDF for a certain channel, based on a clipped histogram
std::map<double, double> computePDF(const std::map<double, int>& clippedChannelHist, int M, double& pmax, double& pmin, bool verbose = false);

// Function to compute the CDF for a certain channel, based on a PDF
std::map<double, double> computeCDF(const std::map<double, double>& PDF);

// Function to compute the weighted histogram distribution (WHD) function and WHDFSum, based on PDF, CDF, pmax, pmin and cMax
std::map<double, double> computeWHDF(const std::map<double, double>& PDF, const std::map<double, double>& CDF, double& WHDFSum, double pmax, double pmin, double cMax, bool verbose = false);

// Function to compute Gammabased on the weighted histogram distribution (WHD) function, WHDFSum and cMax
std::map<double, double> computeGamma(const std::map<double, double>& WHDF, double WHDFSum, double cMax);

// Function to transform a channel, based on the gamma function
cv::Mat transformChannel(const cv::Mat image, int channelIndex, const std::map<double, double> gamma, double cMax, cv::Mat& targetChannel, std::vector<cv::Mat>& otherChannels);

// Function to apply an HSI to BGR transformation
cv::Mat transformHSIToBGR(const cv::Mat& image, int L, const std::string& inputScaleType = "BGR");

// Function to apply the Adaptive Gamma Correction with Weighted Histogram Distribution (AGCWHD) proposed by Veluchamy & Subramani (2019)
void transformAGCWHD(cv::Mat& image, double L, const std::string fileName, const std::string mode, bool verbose = false, const std::string& histPath = "", const std::string& filePath = "");

// Function for histogram plotting from both std::map<double, double> and std::map<double, int>
template <typename T>
void plotHistogram(const std::map<double, T>& histMap, int L, std::string histTitle, std::string histPath, std::string filePath, int&yMax, int& yMid, int offset = 40, bool recalc = true, bool show = false, bool verbose = false) 
{
    int histSize = histMap.size();          // Number of unique intensity values
    const int outputSize = 600;                   // Fixed size for square output canvas
    const int histHeight = outputSize - 2 * offset; // Histogram height, adjusted within the square output
    const int binWidth = cvRound(static_cast<double>(histHeight) / histSize); // Width of each bin based on height

    // Calculate the histogram width based on bin width to maintain the square shape
    const int histWidth = binWidth * histSize;

    // Adjust canvas to be square with padding
    cv::Mat histImage(outputSize, outputSize, CV_8UC3, cv::Scalar(255, 255, 255));

    // Find the max count to normalize histogram heights
    T maxCount = 0;
    for (const auto& pair : histMap) {
        if (pair.second > maxCount) {
            maxCount = pair.second;
        }
    }

    // Calculate yMax and yMid (if requested) and set additional strings
    std::string suffix;
    std::string histTitleDetail;
    if (recalc)
    {
        calculateYAxisLabels(static_cast<int>(maxCount), yMax, yMid);
        suffix = "_HSITransformed";
        histTitleDetail = "Transformed intensity histogram: ";
    }
    else
    {
        suffix = "_HSIOriginal";
        histTitleDetail = "Original intensity histogram: ";
    }

    // Draw histogram bins in dark blue
    int i = 0;
    for (const auto& pair : histMap) {
        int binHeight = cvRound(static_cast<double>(pair.second) / yMax * histHeight);
        cv::rectangle(histImage,
                      cv::Point(i * binWidth + offset, histHeight + offset),  // Shifted starting point
                      cv::Point((i + 1) * binWidth + offset, (histHeight + offset) - binHeight),
                      cv::Scalar(139, 0, 0), cv::FILLED);  // Dark blue color for bars
        i++;
    }

    // Draw X-axis ticks and labels
    int tickInterval = 50;  // Interval for x-axis ticks
    for (int x = 0; x <= histWidth; x += tickInterval * histWidth / L) {
        int intensityValue = (x * L / histWidth);  // Adjusted to map correctly to intensity values
        if (intensityValue % 50 == 0) {  // Draw labels at multiples of 50
            cv::line(histImage, cv::Point(x + offset, histHeight + offset), cv::Point(x + offset, histHeight + offset + 5), cv::Scalar(0, 0, 0), 1);
            cv::putText(histImage, std::to_string(intensityValue), cv::Point(x + offset - 10, histHeight + offset + 20), cv::FONT_HERSHEY_DUPLEX, 0.4, cv::Scalar(0, 0, 0), 1);
        }
    }

    // Add explicit tick marks at 100 and 200
    for (int intensityValue : {100, 200}) {
        int x = intensityValue * histWidth / L;  // Mapping intensity value to x-coordinate
        cv::line(histImage, cv::Point(x + offset, histHeight + offset), cv::Point(x + offset, histHeight + offset + 5), cv::Scalar(0, 0, 0), 1);
        cv::putText(histImage, std::to_string(intensityValue), cv::Point(x + offset - 10, histHeight + offset + 20), cv::FONT_HERSHEY_DUPLEX, 0.4, cv::Scalar(0, 0, 0), 1);
    }

    // Set the tick values based on calculated yMax and yMid
    std::vector<int> yTicks = {0, yMid, yMax}; // Three ticks: 0, yMid, and yMax

    // Draw Y-axis ticks and labels
    for (int frequencyValue : yTicks) {
        // Calculate the Y position for each tick
        int y = (histHeight) - (frequencyValue * (histHeight) / yMax); // Map frequency to Y coordinate
        if (y >= 0 && y <= (histHeight)) { // Ensure y position is within bounds
            cv::line(histImage, cv::Point(offset - 5, y + offset), cv::Point(offset, y + offset), cv::Scalar(0, 0, 0), 1);
            cv::putText(histImage, std::to_string(frequencyValue), cv::Point(5, y + offset + 5), cv::FONT_HERSHEY_DUPLEX, 0.4, cv::Scalar(0, 0, 0), 1);
        }
    }

    // Draw axis rectangle for histogram, fitting it to the histogram's actual width and height
    cv::rectangle(histImage, cv::Point(offset, offset), cv::Point(histWidth + offset, histHeight + offset), cv::Scalar(0, 0, 0), 1);

    // Add X-axis label
    cv::putText(histImage, "Intensity", cv::Point(histWidth / 2 + offset - 20, histHeight + offset + 33), cv::FONT_HERSHEY_DUPLEX, 0.4, cv::Scalar(0, 0, 0), 1);

    // Add title
    cv::putText(histImage, histTitleDetail + filePath, cv::Point(histWidth / 6, 34), cv::FONT_HERSHEY_DUPLEX, 0.6, cv::Scalar(0, 0, 0), 1);

    // Save histogram
    std::string histfilePath = histPath + histTitle + suffix + ".jpg";
    saveImage(histImage, histfilePath, verbose);
    
    // Display histogram
    if (show)
    {
        cv::imshow("Histogram", histImage);
        cv::waitKey(0);
    }
}

#endif