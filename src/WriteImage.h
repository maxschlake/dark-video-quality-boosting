#ifndef WRITE_IMAGE_H
#define WRITE_IMAGE_H

#include <opencv2/opencv.hpp>
#include <string>

class WriteImage
{
public:
    WriteImage();
    bool saveImage(const cv::Mat& image, const std::string& path);
};


#endif