#include <opencv2/opencv.hpp>
#include <iostream>

cv::Mat fitImageToWindow(const cv::Mat& image, int windowMaxWidth, int windowMaxHeight)
{
    // Get image dimensions
    int imageWidth = image.cols;
    int imageHeight = image.rows;

    // Calculate the scaling factor based on the window size
    double scaleFactorWidth = static_cast<double>(windowMaxWidth) / imageWidth;
    double scaleFactorHeight = static_cast<double>(windowMaxWidth) / imageHeight;

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
    cv::Mat channels[3];
    split(image, channels);

    for (int c = 0; c < 3; ++c)
    {
        cv::Mat channel = channels[c];
        
        // find empirical min and max pixel values
        double minVal, maxVal;
        cv::minMaxLoc(channel, &minVal, &maxVal);
        std::cout << minVal << maxVal << "\n";

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
    merge(channels, 3, image);
}