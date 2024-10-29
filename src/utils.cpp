#include <opencv2/opencv.hpp>
#include <iostream>
#include <math.h>
#include <opencv2/imgproc.hpp>
#include <map>
#include <vector>
#include <limits>

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

void stretchColorChannels(const cv::Mat& image, int minLim, int maxLim)
{
    // Loop through each channel
    std::vector<cv::Mat> channels;
    cv::split(image, channels);

    for (auto& channel : channels)
    {
        // find empirical min and max pixel values
        double minVal, maxVal;
        cv::minMaxLoc(channel, &minVal, &maxVal);

        // Stretch the channel
        for (int y = 0; y < image.rows; ++y)
        {
            for (int x = 0; x < image.cols; ++x)
            {
                // Apply stretching formula
                uchar oldVal = channel.at<uchar>(y, x);
                uchar newVal = static_cast<uchar>((oldVal - minVal) * (maxLim - minLim) / (maxVal - minVal) + minLim);
                channel.at<uchar>(y, x) = newVal;
            }
        }
    }

    // Merge the modified channels back together
    cv::merge(channels, image);
}

void transformLogarithmic(const cv::Mat& image, double inputScale, int maxLim)
{
    // Loop through each channel
    std::vector<cv::Mat> channels;
    cv::split(image, channels);

    for (auto& channel : channels)
    {
        // find empirical min and max pixel values
        double minVal, maxVal;
        cv::minMaxLoc(channel, &minVal, &maxVal);

        // Compute the output scale factor
        double outputScale = 255 / (log(1 + maxVal));

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

void transformHistEqual(const cv::Mat& image, double clipLimit = 40.0, cv::Size tileGridSize = cv::Size(8, 8), const std::string& equalType = "local")
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
        std::cerr << "Invalid equalType value. Use 'local' or 'global'.\n";
    }

    // Merge the modified channels back together
    cv::merge(channels, image);
}

cv::Mat transformBGRToHSI(cv::Mat& image, double maxLim, const std::string& scaleType = "normalized")
{      
    int rows = image.rows;
    int cols = image.cols;
    double r, g, b, h, s, i, theta;
    double eps = 1e-6;
    double invMaxLim = 1.0 / maxLim; // Precompute maxLim reciprocal for efficiency

    // Initialize hsiImage depending on the scaleType of the output
    cv::Mat hsiImage;

    if (scaleType == "normalized") {
        hsiImage = cv::Mat(rows, cols, CV_64FC3); // Use CV_64FC3 to store double values
    } else if (scaleType == "BGR") {
        hsiImage = cv::Mat(rows, cols, CV_8UC3); // Use CV_8UC3 to store uchar values
    } else {
        std::cerr << "Invalid scaleType value. Use 'normalized' or 'BGR'." << "\n";
        return cv::Mat(); // Return an empty Mat on error
    }
    
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

            if (scaleType == "normalized")
            {
                // Store HSI values in normalized double format
                hsiImage.at<cv::Vec3d>(row, col)[0] = h;
                hsiImage.at<cv::Vec3d>(row, col)[1] = s;
                hsiImage.at<cv::Vec3d>(row, col)[2] = i;

            }
            else if (scaleType == "BGR")
            {
                // Store HSI values in scaled uchar format of range [0, maxLim]
                hsiImage.at<cv::Vec3b>(row, col)[2] = static_cast<uchar>(h * maxLim);
                hsiImage.at<cv::Vec3b>(row, col)[1] = static_cast<uchar>(s* maxLim);
                hsiImage.at<cv::Vec3b>(row, col)[0] = static_cast<uchar>(i* maxLim);
            }
            else
            {
                std::cerr << "Invalid scaleType value. Use 'normalized' or 'BGR'." << "\n";
            }
        }
    }
    return hsiImage;
}

std::map<double, int> computeChannelHist(const cv::Mat& image, int channelIndex, int L, double& cMax)
{
    cv::Mat channel;
    cv::extractChannel(image, channel, channelIndex);

    // Initialize histogram
    std::map<double, int> channelHist;
    for (int c = 0; c < L; ++c)
    {
        channelHist[c] = 0;
    }

    // Populate the histogram counts with the empirical counts from the channel and collect the maximum value
    cMax = 0;
    for (int row = 0; row < channel.rows; row++)
    {
        for (int col = 0; col < channel.cols; col++)
        {
            double value = channel.at<uchar>(row, col);
            channelHist[value]++;
            cMax = std::max(cMax, static_cast<double>(value));
        }
    }
    std::cout << "Channel Size: " << channel.size() << "\n";
    std::cout << "Unique values in histogram: " << channelHist.size() << "\n";
    std::cout << "cMax: " << cMax << "\n";
    return channelHist;
}

double computeClippingLimit(const std::map<double, int>& channelHist, int L)
{
    int totalValue = 0;
    for (const auto& pair : channelHist)
    {
        totalValue += pair.second;
    }
    double clippingLimit = totalValue / L;
    //std::cout << "Clipping limit: " << clippingLimit << "\n";
    return clippingLimit;
}

std::map<double, double> computeClippedChannelHist(const std::map<double, int>& channelHist, double clippingLimit, int& M)
{
    std::map<double, double> clippedChannelHist;
    M = 0;
    for (auto& pair : channelHist) 
    {
        double value = pair.first;
        double valueCount = static_cast<double>(pair.second);
        //std::cout << value << ", " << valueCount << "\n";
    
        // Clip the count if it exceeds the clipping limit
        double clippedValueCount = (valueCount >= clippingLimit) ? clippingLimit : valueCount;
        clippedChannelHist[value] = clippedValueCount;
        
        M += clippedValueCount;
        //std::cout << clippedChannelHist[value] << "\n";
    }
    return clippedChannelHist;
}

std::map<double, double> computePDF(const std::map<double, double>& clippedChannelHist, int M, double& pmax, double& pmin)
{
    std::map<double, double> PDF;

    pmax = std::numeric_limits<double>::lowest();
    pmin = std::numeric_limits<double>::max();

    for (auto& pair : clippedChannelHist)
    {
        double value = pair.first;
        double probMass = pair.second / static_cast<double>(M);
        PDF[value] = probMass;
        //std::cout << value << ", " << probMass << "\n";
        pmax = std::max(pmax, probMass);
        pmin = std::min(pmin, probMass);
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

std::map<double, double> computeWHDF(const std::map<double, double>& PDF, const std::map<double, double>& CDF, double& WHDFSum, double pmax, double pmin, double cMax)
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
        std::cout << value << ", " << weightedProbMass << "\n";
        std::cout << value << ", " << WHDFSum << "\n";
    }
    return WHDF;
}

// Function to plot a histogram from a value-count map
void plotHistogram(const std::map<double, int>& histMap) {
    int histSize = histMap.size();  // Number of unique intensity values
    int histHeight = 400;           // Height of the histogram image
    int histWidth = 512;            // Width of the histogram image
    int binWidth = cvRound((double)histWidth / histSize);

    // Create a blank image for the histogram plot
    cv::Mat histImage(histHeight, histWidth, CV_8UC3, cv::Scalar(255, 255, 255));

    // Find max count to normalize histogram heights
    int maxCount = 0;
    for (const auto& pair : histMap) {
        if (pair.second > maxCount) {
            maxCount = pair.second;
        }
    }

    // Draw histogram bins
    int i = 0;
    for (const auto& pair : histMap) {
        int binHeight = cvRound((double)pair.second / maxCount * histHeight);
        cv::rectangle(histImage, 
                      cv::Point(i * binWidth, histHeight), 
                      cv::Point((i + 1) * binWidth, histHeight - binHeight), 
                      cv::Scalar(0, 0, 0), 
                      cv::FILLED);
        i++;
    }

    // Display histogram
    cv::imshow("Histogram", histImage);
    cv::waitKey(0);
}

// Function to plot a clipped histogram from a value-count map with double precision
void plotHistogram2(const std::map<double, double>& clippedHistMap) {
    int histSize = clippedHistMap.size();  // Number of unique intensity values
    int histHeight = 400;                  // Height of the histogram image
    int histWidth = 512;                   // Width of the histogram image
    int binWidth = cvRound((double)histWidth / histSize);

    // Create a blank image for the histogram plot
    cv::Mat histImage(histHeight, histWidth, CV_8UC3, cv::Scalar(255, 255, 255));

    // Find max count to normalize histogram heights
    double maxCount = 0.0;
    for (const auto& pair : clippedHistMap) {
        if (pair.second > maxCount) {
            maxCount = pair.second;
        }
    }
    std::cout << "maxCount :" << maxCount << "\n";

    // Draw histogram bins
    int i = 0;
    for (const auto& pair : clippedHistMap) {
        int binHeight = cvRound((pair.second / maxCount) * histHeight);
        cv::rectangle(histImage, 
                      cv::Point(i * binWidth, histHeight), 
                      cv::Point((i + 1) * binWidth, histHeight - binHeight), 
                      cv::Scalar(0, 0, 0), 
                      cv::FILLED);
        i++;
    }

    // Display histogram
    cv::imshow("Clipped Histogram", histImage);
    cv::waitKey(0);
}