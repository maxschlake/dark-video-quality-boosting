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

void transformBGRToHSI(cv::Mat& image, double maxLim)
{      
    int rows = image.rows;
    int cols = image.cols;
    double r, g, b, h, s, i, theta;
    double eps = 1e-6;
    double invMaxLim = 1.0 / maxLim; // Precompute maxLim reciprocal for efficiency
    
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
            h = h * 180.0 / CV_PI;  // convert from radians to degrees
            h /= 360.0;  // normalize

            // Intensity
            i = (BGRsum) / 3.0;
            
            // Saturation
            s = 1 - (3.0 * minVal / (BGRsum + eps));

            // Scale HSI to the [0, maxLim] range
            h = h * maxLim;
            s = s * maxLim;
            i = i * maxLim;

            // Replace RGB by HSI
            image.at<cv::Vec3b>(row, col)[2] = static_cast<uchar>(h);
            image.at<cv::Vec3b>(row, col)[1] = static_cast<uchar>(s);
            image.at<cv::Vec3b>(row, col)[0] = static_cast<uchar>(i);
        }
    }
}

cv::Mat test(cv::Mat& image)
{
    cv::Mat hsi(image.rows, image.cols, image.type());

    float r, g, b, h, s, in;

    for(int i = 0; i < image.rows; i++)
        {
        for(int j = 0; j < image.cols; j++)
            {
            b = image.at<cv::Vec3b>(i, j)[0];
            g = image.at<cv::Vec3b>(i, j)[1];
            r = image.at<cv::Vec3b>(i, j)[2];

            in = (b + g + r) / 3;

            int min_val = 0;
            min_val = std::min(r, std::min(b,g));

            s = 1 - 3*(min_val/(b + g + r));
            if(s < 0.00001)
                {
                    s = 0;
                }else if(s > 0.99999){
                    s = 1;
                }

            if(s != 0)
                {
                h = 0.5 * ((r - g) + (r - b)) / sqrt(((r - g)*(r - g)) + ((r - b)*(g - b)));
                h = acos(h);

                if(b <= g)
                    {
                    h = h;
                    } else{
                    h = ((360 * 3.14159265) / 180.0) - h;
                    }
                }

            hsi.at<cv::Vec3b>(i, j)[0] = (h * 180) / 3.14159265;
            hsi.at<cv::Vec3b>(i, j)[1] = s*100;
            hsi.at<cv::Vec3b>(i, j)[2] = in;
            }
        }

    return hsi;
}