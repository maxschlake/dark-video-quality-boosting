#include "WriteImage.h"
#include <filesystem>
#include <iostream>

WriteImage::WriteImage(){}

bool WriteImage::saveImage(const cv::Mat& image, const std::string& path)
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
        std::cout << "Image successfully saved under: " << path << "\n";
        return true;
    }
    else
    {
        std::cerr << "Error: Image could not be saved." << "\n";
        return false;
    }
}