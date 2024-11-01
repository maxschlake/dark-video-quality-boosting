#include <opencv2/opencv.hpp>
#include <iostream>
#include <filesystem>
#include <math.h>
#include <opencv2/imgproc.hpp>
#include <map>
#include <vector>
#include <limits>
#include "utils.h"

bool saveImage(const cv::Mat& image, const std::string& path, bool verbose)
{   
    // Get the directory from the provided path
    std::filesystem::path dirPath = std::filesystem::path(path).parent_path();

    // Check if the directory exists, if not create it
    if (!std::filesystem::exists(dirPath))
    {
        std::filesystem::create_directories(dirPath);
    }
    if (cv::imwrite(path, image))
    {
        if (verbose)
        {
            std::cout << "Image successfully saved under: " << path << "\n";
        }
        
        return true;
    }
    else
    {
        std::cerr << "Error: Image could not be saved." << "\n";
        return false;
    }
}

void calculateYAxisLabels(int maxCount, int& yMax, int& yMid) 
{
    int maxCountCopy = maxCount;
    int counter = 0;
    while(maxCountCopy)
    {        
        maxCountCopy = maxCountCopy / 10;
        counter++;
    }
    double factor = pow(10, counter - 1);
    int yMaxIntermediate = maxCount + factor;

    // Determine final yMax and yMid based on yMaxIntermediate and the rounding factor
    yMax = static_cast<int>(std::floor(static_cast<double>(yMaxIntermediate) / factor)) * factor;
    yMid = yMax / 2;
}

cv::Mat fitImageToWindow(const cv::Mat& image, int windowMaxWidth, int windowMaxHeight)
{
    // Get image dimensions
    int imageWidth = image.cols;
    int imageHeight = image.rows;

    // Calculate the scaling factor based on the window size
    double scaleFactorWidth = static_cast<double>(windowMaxWidth) / imageWidth;
    double scaleFactorHeight = static_cast<double>(windowMaxHeight) / imageHeight;

    // Use the smaller of the two scaling factors to ensure the image fits both dimensions
    double scaleFactor = std::min(scaleFactorWidth, scaleFactorHeight);

    // If the image fits within the window, do not resize
    if (scaleFactor >= 1.0)
    {
        return image.clone();
    }

    // Otherwise, resize the image
    cv::Mat resizedImage;
    cv::resize(image, resizedImage, cv::Size(), scaleFactor, scaleFactor);
    return resizedImage;
}

void stretchColorChannels(const cv::Mat& image, int minL, int L)
{
    int maxL = L - 1;
    
    // Loop through each channel
    std::vector<cv::Mat> channels;
    cv::split(image, channels);

    for (auto& channel : channels)
    {
        // find empirical min and max pixel values
        double minVal, maxVal;
        cv::minMaxLoc(channel, &minVal, &maxVal);
        double valRange = maxVal - minVal;

        // Stretch the channel
        for (int y = 0; y < image.rows; ++y)
        {
            for (int x = 0; x < image.cols; ++x)
            {
                // Apply stretching formula
                uchar oldVal = channel.at<uchar>(y, x);
                uchar newVal = static_cast<uchar>((oldVal - minVal) * (maxL - minL) / valRange + minL);
                channel.at<uchar>(y, x) = newVal;
            }
        }
    }

    // Merge the modified channels back together
    cv::merge(channels, image);
}

void transformLogarithmic(const cv::Mat& image, double inputScale, int L)
{
    int maxL = L - 1;
    
    // Loop through each channel
    std::vector<cv::Mat> channels;
    cv::split(image, channels);

    for (auto& channel : channels)
    {
        // Find empirical min and max pixel values
        double minVal, maxVal;
        cv::minMaxLoc(channel, &minVal, &maxVal);

        // Compute the output scale factor
        double outputScale = maxL / (log(1 + maxVal));

        // Stretch the channel
        for (int y = 0; y < image.rows; ++y)
        {
            for (int x = 0; x < image.cols; ++x)
            {
                // Apply stretching formula
                uchar oldVal = channel.at<uchar>(y, x);
                uchar newVal = static_cast<uchar>(outputScale * log(1 + (exp(inputScale) - 1) * oldVal));
                channel.at<uchar>(y, x) = newVal;
            }
        }
    }
    // Merge the modified channels back together
    cv::merge(channels, image);
}

void transformHistEqual(const cv::Mat& image, double clipLimit, cv::Size tileGridSize, const std::string& equalType)
{
    // Apply channel-wise
    std::vector<cv::Mat> channels;
    split(image, channels);

    if (equalType == "local")
    {
        cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(clipLimit, tileGridSize);
        for (auto& channel : channels)
        {
            clahe->apply(channel, channel);
        }
    }
    else if (equalType == "global")
    {
        for (auto& channel : channels)
        {
            cv::equalizeHist(channel, channel);
        }
    }
    else
    {
        std::cerr << "Error: Invalid equalType value. Use 'local' or 'global'.\n";
    }
    // Merge the modified channels back together
    cv::merge(channels, image);
}

cv::Mat transformBGRToHSI(cv::Mat& image, int L, const std::string& outputScaleType)
{      
    int maxL = L - 1;
    
    int rows = image.rows;
    int cols = image.cols;
    double r, g, b, h, s, i, theta;
    double eps = 1e-6;
    double invMaxLim = 1.0 / maxL; // Precompute maxLim reciprocal for efficiency

    // Initialize hsiImage depending on the scaleType of the output
    cv::Mat hsiImage;
    int matType = (outputScaleType == "normalized") ? CV_64FC3 : CV_8UC3;
    hsiImage = cv::Mat(rows, cols, matType);

    // Loop through each pixel
    for (int row = 0; row < rows; ++row)
    {
        for (int col = 0; col < cols; ++col)
        {
            b = image.at<cv::Vec3b>(row, col)[0] * invMaxLim;
            g = image.at<cv::Vec3b>(row, col)[1] * invMaxLim;
            r = image.at<cv::Vec3b>(row, col)[2] * invMaxLim;

            double BGRsum = b + g + r;
            double minVal = std::min(b, std::min(g, r));

            // Hue
            theta = acos((0.5 * ((r - g) + (r - b))) / (sqrt((r - g) * (r - g) + (r - b) * (g - b)) + eps));
            h = (b <= g) ? theta : (2 * CV_PI - theta);
            h = h * 180.0 / CV_PI;  // Convert from radians to degrees
            h /= 360.0;  // Normalize

            // Intensity
            i = (BGRsum) / 3.0;
            
            // Saturation
            s = 1 - (3.0 * minVal / (BGRsum + eps));

            if (outputScaleType == "normalized")
            {
                // Store HSI values in double format of range [0, 1]
                hsiImage.at<cv::Vec3d>(row, col)[2] = h;
                hsiImage.at<cv::Vec3d>(row, col)[1] = s;
                hsiImage.at<cv::Vec3d>(row, col)[0] = i;

            }
            else if (outputScaleType == "BGR")
            {
                // Store HSI values in uchar format of range [0, maxLim]
                hsiImage.at<cv::Vec3b>(row, col)[2] = static_cast<uchar>(h * maxL);
                hsiImage.at<cv::Vec3b>(row, col)[1] = static_cast<uchar>(s* maxL);
                hsiImage.at<cv::Vec3b>(row, col)[0] = static_cast<uchar>(i* maxL);
            }
            else
            {
                std::cerr << "Error: Invalid scaleType value. Use 'normalized' or 'BGR'." << "\n";
            }
        }
    }
    return hsiImage;
}

std::map<double, int> computeChannelHist(const cv::Mat& image, int channelIndex, int L, double& cMax, cv::Mat& targetChannel, std::vector<cv::Mat>& otherChannels, bool verbose)
{
    // Extract the target channel
    cv::extractChannel(image, targetChannel, channelIndex);

    // Initialize histogram for the target channel
    std::map<double, int> channelHist;
    for (int c = 0; c < L; ++c)
    {
        channelHist[c] = 0;
    }

    // Populate the histogram counts with the empirical counts from the target channel and collect the maximum value
    cMax = 0;
    for (int row = 0; row < targetChannel.rows; row++)
    {
        for (int col = 0; col < targetChannel.cols; col++)
        {
            double value = targetChannel.at<uchar>(row, col);
            channelHist[value]++;
            cMax = std::max(cMax, static_cast<double>(value));
            //std::cout << "value: "<< value << ", valueCount: " << channelHist[value] << "\n";
        }
    }
    if (verbose)
    {
        std::cout << "Target channel size: " << targetChannel.size() << "\n";
        std::cout << "Unique values in original histogram: " << channelHist.size() << "\n";
        std::cout << "cMax: " << cMax << "\n";
    }


    // Extract and store the remaining channels in the otherChannels vector
    otherChannels.clear();
    for (int c = 0; c < image.channels(); ++c)
    {
        if (c != channelIndex)
        {
            cv::Mat otherChannel;
            cv::extractChannel(image, otherChannel, c);
            otherChannels.push_back(otherChannel);
        }
    }

    return channelHist;
}

double computeClippingLimit(const std::map<double, int>& channelHist, int L, bool verbose)
{
    int totalValue = 0;
    for (const auto& pair : channelHist)
    {
        totalValue += pair.second;
    }
    double clippingLimit = totalValue / L;
    if (verbose)
    {
        std::cout << "Clipping limit: " << clippingLimit << "\n";
    }
    return clippingLimit;
}

std::map<double, double> computeClippedChannelHist(const std::map<double, int>& channelHist, double clippingLimit, int& M, bool verbose)
{
    std::map<double, double> clippedChannelHist;
    M = 0;
    for (auto& pair : channelHist) 
    {
        double value = pair.first;
        double valueCount = static_cast<double>(pair.second);
    
        // Clip the count if it exceeds the clipping limit
        double clippedValueCount = (valueCount >= clippingLimit) ? clippingLimit : valueCount;
        clippedChannelHist[value] = clippedValueCount;
        M += clippedValueCount;
        //std::cout << "value: " << value << ", valueCount: " << valueCount << ", clippedValueCount: " << clippedValueCount << "\n";
    }
    if (verbose)
    {
        std::cout << "M: " << M << "\n";
        std::cout << "Unique values in clipped histogram: " << clippedChannelHist.size() << "\n";
    }
    return clippedChannelHist;
}

std::map<double, double> computePDF(const std::map<double, double>& clippedChannelHist, int M, double& pmax, double& pmin, bool verbose)
{
    std::map<double, double> PDF;

    pmax = std::numeric_limits<double>::lowest();
    pmin = std::numeric_limits<double>::max();

    for (auto& pair : clippedChannelHist)
    {
        double value = pair.first;
        double probMass = pair.second / static_cast<double>(M);
        PDF[value] = probMass;
        pmax = std::max(pmax, probMass);
        pmin = std::min(pmin, probMass);
        //std::cout << "value: " << value << ", probMass: " << probMass << "\n";
    }
    if (verbose)
    {
        std::cout << "pmax: " << pmax << ", pmin: " << pmin <<"\n";
    }
    return PDF;
}

std::map<double, double> computeCDF(const std::map<double, double>& PDF)
{
    std::map<double, double> CDF;
    double cumProbMass = 0;
    for (auto& pair : PDF)
    {
        double value = pair.first;
        cumProbMass += pair.second;
        CDF[value] = cumProbMass;
        //std::cout << value << ", " << cumProbMass << "\n";
    }
    return CDF;
}

std::map<double, double> computeWHDF(const std::map<double, double>& PDF, const std::map<double, double>& CDF, double& WHDFSum, double pmax, double pmin, double cMax, bool verbose)
{
    double pRange = pmax - pmin;
    std::map<double, double> WHDF;
    WHDFSum = 0.0;
    for (auto& pair : PDF)
    {
        double value = pair.first;
        double probMass = pair.second;
        double alpha = CDF.at(value);
        double weightedProbMass = pmax * pow(((probMass - pmin) / (pRange)), alpha);
        WHDF[value] = weightedProbMass;
        if (value < cMax)
        {
            WHDFSum += weightedProbMass;
        }
        //std::cout << "value:" << value << ", probMass: " << probMass << ", alpha: " << alpha << ", weightedProbMass: " << weightedProbMass << "\n";
    }
    if (verbose)
    {
        std::cout << "WHDFSum: " << WHDFSum << "\n";
    }
    return WHDF;
}

std::map<double, double> computeGamma(const std::map<double, double>& WHDF, double WHDFSum, double cMax)
{
    // Compute weighted CDF
    std::map<double, double> gamma;
    double cumWHDF = 0.0;
    for (auto& pair : WHDF)
    {
        double value = pair.first;
        if (value < cMax)
        {
            double weightedProbMass = pair.second;
            cumWHDF += weightedProbMass / WHDFSum;
            double gammaMass = std::max(0.0, 1 - cumWHDF);
            gamma[value] = gammaMass;
            //std::cout << "value: " << value << ", gammaMass: " << gammaMass << ", cumWHDF: " << cumWHDF << "\n";
        }
    }
    return gamma;
}

cv::Mat transformChannel(cv::Mat image, int channelIndex, std::map<double, double> gamma, double cMax, cv::Mat& targetChannel, std::vector<cv::Mat>& otherChannels)
{
    int rows = targetChannel.rows;
    int cols = targetChannel.cols;

    for (int row = 0; row < rows; ++row)
    {
        for (int col = 0; col < cols; ++col)
        {
            double value = targetChannel.at<uchar>(row, col);
            double transformedValue = round(pow((value / cMax), gamma[value]) * cMax);
            targetChannel.at<uchar>(row, col) = static_cast<uchar>(transformedValue);
            //std::cout << "value: " << value << ", gammaMass: " << gamma[value] << ", transformedValue: " << transformedValue << "\n";
        }
    }

    // Merge the transformed target channel with the other channels in the original order
    std::vector<cv::Mat> channels;
    for (int c = 0; c < image.channels(); ++c)
    {
        if (c == channelIndex)
        {
            channels.push_back(targetChannel);
        }
        else
        {
            channels.push_back(otherChannels[c < channelIndex ? c : c - 1]);
        }
    }

    cv::Mat transformedImage;
    cv::merge(channels, transformedImage);
    return transformedImage;
}

cv::Mat transformHSIToBGR(cv::Mat& image, int L, const std::string& inputScaleType) 
{
    int maxL = L - 1;
    int rows = image.rows;
    int cols = image.cols;
    double r, g, b, h, s, i;

    // Initialize bgrImage and obtain scale factor depending on the scaleType of the input
    cv::Mat bgrImage = cv::Mat(rows, cols, CV_8UC3);
    double scaleFactor = (inputScaleType == "normalized") ? 1.0 : 1.0 / maxL;

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            h = image.at<cv::Vec3b>(row, col)[2] * scaleFactor * 360.0;     // Hue [0, 360]
            s = image.at<cv::Vec3b>(row, col)[1] * scaleFactor;             // Saturation [0, 1]
            i = image.at<cv::Vec3b>(row, col)[0] * scaleFactor;             // Intensity [0, 1]
            //std::cout << "h: " << h << ", s: " << s << ", i: " << i << "\n";

            double h2 = 0;
            double convFactor = CV_PI / 180.0;

            if (h < 120) 
            {   // RG Sector
                b = i * (1 - s);
                r = i * (1 + (s * cos(h * convFactor) / cos((60 - h) * convFactor)));
                g = 3 * i - (r + b);
            } 
            else if (h < 240) 
            {   // GB Sector
                h2 = h - 120;
                r = i * (1 - s);
                g = i * (1 + (s * cos(h2 * convFactor) / cos((60 - h2) * convFactor)));
                b = 3 * i - (r + g);
            } 
            else 
            {   // BR Sector
                h2 = h - 240;
                g = i * (1 - s);
                b = i * (1 + (s * cos(h2 * convFactor) / cos((60 - h2) * convFactor)));
                r = 3 * i - (g + b);
            }

            // Store BGR values in uchar format of range [0, maxLim]
            bgrImage.at<cv::Vec3b>(row, col)[0] = static_cast<uchar>(std::clamp(b, 0.0, 1.0) * maxL);
            bgrImage.at<cv::Vec3b>(row, col)[1] = static_cast<uchar>(std::clamp(g, 0.0, 1.0) * maxL);
            bgrImage.at<cv::Vec3b>(row, col)[2] = static_cast<uchar>(std::clamp(r, 0.0, 1.0) * maxL);
        }
    }
   return bgrImage;
}

cv::Mat transformAGCWHD(cv::Mat& image, double L, std::string fileName, std::string histPath, std::string filePath, bool verbose)
{
    int channelIndex = 0;
    double cMax;
    cv::Mat originalTargetChannel;
    cv::Mat transformedTargetChannel;
    std::vector<cv::Mat> originalOtherChannels;
    std::vector<cv::Mat> transformedOtherChannels;
    int M;
    double pmax, pmin;
    double WHDFSum;
    int yMax, yMid;

    cv::Mat originalHSIImage = transformBGRToHSI(image, L, "BGR");
    std::map<double, int> originalHSIHist = computeChannelHist(originalHSIImage, channelIndex, L, cMax, originalTargetChannel, originalOtherChannels, verbose);
    double clippingLimit = computeClippingLimit(originalHSIHist, L, verbose);
    std::map<double, double> clippedHSIHist = computeClippedChannelHist(originalHSIHist, clippingLimit, M, verbose);
    std::map<double, double> PDF = computePDF(clippedHSIHist, M, pmax, pmin, verbose);
    std::map<double, double> CDF = computeCDF(PDF);
    std::map<double, double> WHDF = computeWHDF(PDF, CDF, WHDFSum, pmax, pmin, cMax, verbose);
    std::map<double, double> gamma = computeGamma(WHDF, WHDFSum, cMax);
    cv::Mat transformedHSIImage = transformChannel(originalHSIImage, channelIndex, gamma, cMax, originalTargetChannel, originalOtherChannels);
    std::map<double, int> transformedHSIHist = computeChannelHist(transformedHSIImage, channelIndex, L, cMax, transformedTargetChannel, transformedOtherChannels);
    cv::Mat transformedBGRImage = transformHSIToBGR(transformedHSIImage, L, "BGR");
    plotHistogram(transformedHSIHist, L, fileName, histPath, filePath, yMax, yMid, 40, true, false, verbose);
    plotHistogram(originalHSIHist, L, fileName, histPath, filePath, yMax, yMid, 40, false, false, verbose);

    return transformedBGRImage;
}
