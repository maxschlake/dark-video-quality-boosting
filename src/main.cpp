#include <QApplication>
#include "ReadImageQt.h"
#include <opencv2/opencv.hpp>
#include "utils.h"
#include <opencv2/imgproc.hpp>

int main (int argc, char *argv[])
{
    const std::string fileName = "jam";
    const std::string fileType = "jpg";
    const int L = 256;
    const bool verbose = true;
    const std::string filePath = fileName + "." + fileType;
    const std::string rawPath = "images/raw/";
    const std::string modPath = "images/mod/";
    const std::string histPath = "images/hist/";
    const std::string rawImagePath = rawPath + filePath;
    const std::string modImagePath = modPath + filePath;
    
    // READING
    /*
    QApplication app(argc, argv);
    ReadImageQt readImageQt;
    readImageQt.showImage(rawImagePath);
    readImageQt.show();
    return app.exec();
    */
   
    // PROCESSING
    cv::Mat image = cv::imread(rawImagePath);

    if(image.empty())
    {
        std::cerr << "Image could not be loaded" << "\n";
        return -1;
    }

    image = fitImageToWindow(image, 1280, 720);

    stretchColorChannels(image, 0, L);

    //transformLogarithmic(image, 0.2, L);

    //transformHistEqual(image, 2.0, cv::Size (8,8), "local");

    transformAGCWHD(image, L, fileName, histPath, filePath, verbose);

    saveImage(image, modImagePath, verbose);
    QApplication app(argc, argv);
    ReadImageQt readImageQt;
    readImageQt.showImage(QString::fromStdString(modImagePath));
    readImageQt.show();
    return app.exec();
}