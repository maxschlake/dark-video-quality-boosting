#include <opencv2/opencv.hpp>
#include <iostream>
#include <math.h>
#include <opencv2/imgproc.hpp>

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

std::map<double, int> computeChannelHist(const cv::Mat& image, int channelIndex)
{
    cv::Mat channel;
    cv::extractChannel(image, channel, channelIndex);
    std::map<double, int> valueCount;
    for (int row = 0; row < channel.rows; row++)
    {
        for (int col = 0; col < channel.cols; col++)
        {
            double value = channel.at<double>(row, col);
            valueCount[value]++;
        }
    }
    
    return valueCount;
}

double computeClippingLimit(const std::map<double, int>& channelHist, int maxLim)
{
    int totalValue = 0;
    for (const auto& pair : channelHist)
    {
        totalValue += pair.second;
    }
    double clippingLimit = totalValue / maxLim;
    return clippingLimit;
}

std::map<double, double> computeClippedChannelHist(const std::map<double, int>& channelHist, double clippingLimit)
{
    std::map<double, double> clippedChannelHist;
    for (auto& pair : channelHist) 
    {
        if (pair.second >= clippingLimit) 
        {
            double value = pair.first;
            double valueCount = static_cast<double>(pair.second);
        
            // Clip the count if it exceeds the clipping limit
            clippedChannelHist[value] = (valueCount >= clippingLimit) ? clippingLimit : valueCount;
        }
    }
    return clippedChannelHist;
}