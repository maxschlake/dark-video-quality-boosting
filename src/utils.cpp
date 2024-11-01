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
    const std::filesystem::path dirPath = std::filesystem::path(path).parent_path();

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
    const double factor = pow(10, counter - 1);
    const int yMaxIntermediate = maxCount + factor;

    // Determine final yMax and yMid based on yMaxIntermediate and the rounding factor
    yMax = static_cast<int>(std::floor(static_cast<double>(yMaxIntermediate) / factor)) * factor;
    yMid = yMax / 2;
}

cv::Mat fitImageToWindow(const cv::Mat& image, int windowMaxWidth, int windowMaxHeight)
{
    // Get image dimensions
    const int imageWidth = image.cols;
    const int imageHeight = image.rows;

    // Calculate the scaling factor based on the window size
    const double scaleFactorWidth = static_cast<double>(windowMaxWidth) / imageWidth;
    const double scaleFactorHeight = static_cast<double>(windowMaxHeight) / imageHeight;

    // Use the smaller of the two scaling factors to ensure the image fits both dimensions
    const double scaleFactor = std::min(scaleFactorWidth, scaleFactorHeight);

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
    const int maxL = L - 1;
    
    // Loop through each channel
    std::vector<cv::Mat> channels;
    cv::split(image, channels);

    for (cv::Mat& channel : channels)
    {
        // find empirical min and max pixel values
        double minVal, maxVal;
        cv::minMaxLoc(channel, &minVal, &maxVal);
        const double valRange = maxVal - minVal;

        // Stretch the channel
        for (int y = 0; y < image.rows; ++y)
        {
            for (int x = 0; x < image.cols; ++x)
            {
                // Apply stretching formula
                const uchar oldVal = channel.at<uchar>(y, x);
                const uchar newVal = static_cast<uchar>((oldVal - minVal) * (maxL - minL) / valRange + minL);
                channel.at<uchar>(y, x) = newVal;
            }
        }
    }

    // Merge the modified channels back together
    cv::merge(channels, image);
}

void transformLogarithmic(const cv::Mat& image, double inputScale, int L)
{
    const int maxL = L - 1;
    
    // Loop through each channel
    std::vector<cv::Mat> channels;
    cv::split(image, channels);

    for (cv::Mat& channel : channels)
    {
        // Find empirical min and max pixel values
        double minVal, maxVal;
        cv::minMaxLoc(channel, &minVal, &maxVal);

        // Compute the output scale factor
        const double outputScale = maxL / (log(1 + maxVal));

        // Stretch the channel
        for (int y = 0; y < image.rows; ++y)
        {
            for (int x = 0; x < image.cols; ++x)
            {
                // Apply stretching formula
                const uchar oldVal = channel.at<uchar>(y, x);
                const uchar newVal = static_cast<uchar>(outputScale * log(1 + (exp(inputScale) - 1) * oldVal));
                channel.at<uchar>(y, x) = newVal;
            }
        }
    }
    // Merge the modified channels back together
    cv::merge(channels, image);
}

void transformHistEqual(const cv::Mat& image, double clipLimit, const cv::Size tileGridSize, const std::string& equalType)
{
    // Apply channel-wise
    std::vector<cv::Mat> channels;
    split(image, channels);

    if (equalType == "local")
    {
        cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(clipLimit, tileGridSize);
        for (cv::Mat& channel : channels)
        {
            clahe->apply(channel, channel);
        }
    }
    else if (equalType == "global")
    {
        for (cv::Mat& channel : channels)
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

cv::Mat transformBGRToHSI(const cv::Mat& image, int L, const std::string& outputScaleType)
{      
    const int maxL = L - 1;
    
    const int rows = image.rows;
    const int cols = image.cols;
    double r, g, b, h, s, i, theta;
    const double eps = 1e-6;
    const double invMaxLim = 1.0 / maxL; // Precompute maxLim reciprocal for efficiency

    // Initialize hsiImage depending on the scaleType of the output
    cv::Mat hsiImage;
    const int matType = (outputScaleType == "normalized") ? CV_64FC3 : CV_8UC3;
    hsiImage = cv::Mat(rows, cols, matType);

    // Loop through each pixel
    for (int row = 0; row < rows; ++row)
    {
        for (int col = 0; col < cols; ++col)
        {
            b = image.at<cv::Vec3b>(row, col)[0] * invMaxLim;
            g = image.at<cv::Vec3b>(row, col)[1] * invMaxLim;
            r = image.at<cv::Vec3b>(row, col)[2] * invMaxLim;

            const double BGRsum = b + g + r;
            const double minVal = std::min(b, std::min(g, r));

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
    cMax = 0.0;
    for (int row = 0; row < targetChannel.rows; row++)
    {
        for (int col = 0; col < targetChannel.cols; col++)
        {
            const double value = targetChannel.at<uchar>(row, col);
            channelHist[value]++;
            cMax = std::max(cMax, value);
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
    double clippingLimit = static_cast<double>(totalValue) / L;
    if (verbose)
    {
        std::cout << "Clipping limit: " << clippingLimit << "\n";
    }
    return clippingLimit;
}

std::map<double, int> computeClippedChannelHist(const std::map<double, int>& channelHist, double clippingLimit, int& M, bool verbose)
{
    std::map<double, int> clippedChannelHist;
    M = 0;
    for (const auto& pair : channelHist) 
    {
        const double value = pair.first;
        const double valueCount = pair.second;
    
        // Clip the count if it exceeds the clipping limit
        const int clippedValueCount = (valueCount >= clippingLimit) ? static_cast<int>(clippingLimit) : valueCount;
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

std::map<double, double> computePDF(const std::map<double, int>& clippedChannelHist, int M, double& pmax, double& pmin, bool verbose)
{
    std::map<double, double> PDF;

    pmax = std::numeric_limits<double>::lowest();
    pmin = std::numeric_limits<double>::max();

    for (const auto& pair : clippedChannelHist)
    {
        const double value = pair.first;
        const double probMass = pair.second / static_cast<double>(M);
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
    for (const auto& pair : PDF)
    {
        const double value = pair.first;
        cumProbMass += pair.second;
        CDF[value] = cumProbMass;
        //std::cout << value << ", " << cumProbMass << "\n";
    }
    return CDF;
}

std::map<double, double> computeWHDF(const std::map<double, double>& PDF, const std::map<double, double>& CDF, double& WHDFSum, double pmax, double pmin, double cMax, bool verbose)
{
    const double pRange = pmax - pmin;
    std::map<double, double> WHDF;
    WHDFSum = 0.0;
    for (const auto& pair : PDF)
    {
        const double value = pair.first;
        const double probMass = pair.second;
        const double alpha = CDF.at(value);
        const double weightedProbMass = pmax * pow(((probMass - pmin) / (pRange)), alpha);
        WHDF[value] = weightedProbMass;
        if (value <= cMax)
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
    for (const auto& pair : WHDF)
    {
        const double value = pair.first;
        if (value <= cMax)
        {
            const double weightedProbMass = pair.second;
            cumWHDF += weightedProbMass / WHDFSum;
            const double gammaMass = std::max(0.0, 1 - cumWHDF);
            gamma[value] = gammaMass;
            //std::cout << "value: " << value << ", gammaMass: " << gammaMass << ", cumWHDF: " << cumWHDF << "\n";
        }
    }
    return gamma;
}

cv::Mat transformChannel(const cv::Mat image, int channelIndex, const std::map<double, double> gamma, double cMax, cv::Mat& targetChannel, std::vector<cv::Mat>& otherChannels)
{
    const int rows = targetChannel.rows;
    const int cols = targetChannel.cols;

    for (int row = 0; row < rows; ++row)
    {
        for (int col = 0; col < cols; ++col)
        {
            const double value = targetChannel.at<uchar>(row, col);
            auto gammaIter = gamma.find(value);

            // Output value and corresponding gamma pair if it exists
            if (gammaIter != gamma.end())
            {
                const double transformedValue = round(pow((value / cMax), gammaIter->second) * cMax);
                targetChannel.at<uchar>(row, col) = static_cast<uchar>(transformedValue);
                //std::cout << "value: " << value << ", gammaIter->first: " << gammaIter->first << ", gammaIter->second: " << gammaIter->second << "\n";
            }
            else
            {
                std::cerr << "Error: Lookup value " << value << " could not be found in gamma function.\n";
            }
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

cv::Mat transformHSIToBGR(const cv::Mat& image, int L, const std::string& inputScaleType) 
{
    const int maxL = L - 1;
    const int rows = image.rows;
    const int cols = image.cols;

    // Initialize bgrImage and obtain scale factor depending on the scaleType of the input
    cv::Mat bgrImage = cv::Mat(rows, cols, CV_8UC3);
    const double scaleFactor = (inputScaleType == "normalized") ? 1.0 : 1.0 / maxL;

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            const double h = image.at<cv::Vec3b>(row, col)[2] * scaleFactor * 360.0;     // Hue [0, 360]
            const double s = image.at<cv::Vec3b>(row, col)[1] * scaleFactor;             // Saturation [0, 1]
            const double i = image.at<cv::Vec3b>(row, col)[0] * scaleFactor;             // Intensity [0, 1]
            //std::cout << "h: " << h << ", s: " << s << ", i: " << i << "\n";

            double r, g, b;
            double h2 = 0;
            const double convFactor = CV_PI / 180.0;

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

void transformAGCWHD(cv::Mat& image, double L, const std::string fileName, const std::string histPath, const std::string filePath, bool verbose)
{
    const int channelIndex = 0;
    double cMax;
    cv::Mat originalTargetChannel;
    cv::Mat transformedTargetChannel;
    std::vector<cv::Mat> originalOtherChannels;
    std::vector<cv::Mat> transformedOtherChannels;
    int M;
    double pmax, pmin;
    double WHDFSum;
    int yMax, yMid;

    const cv::Mat originalHSIImage = transformBGRToHSI(image, L, "BGR");
    const std::map<double, int> originalHSIHist = computeChannelHist(originalHSIImage, channelIndex, L, cMax, originalTargetChannel, originalOtherChannels, verbose);
    double clippingLimit = computeClippingLimit(originalHSIHist, L, verbose);
    const std::map<double, int> clippedHSIHist = computeClippedChannelHist(originalHSIHist, clippingLimit, M, verbose);
    const std::map<double, double> PDF = computePDF(clippedHSIHist, M, pmax, pmin, verbose);
    const std::map<double, double> CDF = computeCDF(PDF);
    const std::map<double, double> WHDF = computeWHDF(PDF, CDF, WHDFSum, pmax, pmin, cMax, verbose);
    const std::map<double, double> gamma = computeGamma(WHDF, WHDFSum, cMax);
    cv::Mat transformedHSIImage = transformChannel(originalHSIImage, channelIndex, gamma, cMax, originalTargetChannel, originalOtherChannels);
    const std::map<double, int> transformedHSIHist = computeChannelHist(transformedHSIImage, channelIndex, L, cMax, transformedTargetChannel, transformedOtherChannels);
    image = transformHSIToBGR(transformedHSIImage, L, "BGR");
    plotHistogram(transformedHSIHist, L, fileName, histPath, filePath, yMax, yMid, 40, true, false, verbose);
    plotHistogram(originalHSIHist, L, fileName, histPath, filePath, yMax, yMid, 40, false, false, verbose);
}
