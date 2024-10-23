# include "WriteImage.h"

WriteImage::WriteImage()
{
    //
}

bool WriteImage::saveImage(const cv::Mat& image, const std::string& path)
{
    if (cv::imwrite(path, image))
    {
        return true;
    }
    else
    {
        return false;
    }
}