#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <iostream>

int main()
{
    std::string imagePath = "C:/Users/maxim/Documents/Projects/video-quality-boosting/images/penguin.jpg"; // Absolute path
    cv::Mat image = cv::imread(imagePath, cv::IMREAD_COLOR);
    cv::imshow("Display window", image);
    cv::waitKey(0);
    return 0;
}