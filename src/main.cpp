#include <QApplication>
#include "ReadImageQt.h"
#include "WriteImage.h"
#include <opencv2/opencv.hpp>
#include "utils.h"
#include <opencv2/imgproc.hpp>

int main (int argc, char *argv[])
{
    std::string fileName = "path";
    std::string fileType = "jpg";
    std::string filePath = fileName + "." + fileType;
    std::string rawPath = "images/raw/";
    std::string modPath = "images/mod/";
    std::string histPath = "images/hist/";
    std::string rawImagePath = rawPath + filePath;
    std::string modImagePath = modPath + filePath;
    
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
    std::cerr << "Image could not be loaded" << "\n";

    image = fitImageToWindow(image, 1280, 720);

    const double L = 256.0;
    //stretchColorChannels(image, 0, L - 1);

    //transformLogarithmic(image, 0.2, L - 1);

    //transformHistEqual(image, 2.0, cv::Size (8,8), "local");

    cv::Mat transformedImage = transformAGCWHD(image, L, fileName, histPath, filePath);

    WriteImage writer;
    bool success = writer.saveImage(transformedImage, modImagePath);
    QApplication app(argc, argv);
    ReadImageQt readImageQt;
    readImageQt.showImage(QString::fromStdString(modImagePath));
    readImageQt.show();
    return app.exec();

    // WRITING
    /*
    WriteImage writer;
    bool success = writer.saveImage(image, modImagePath);
    return 0;
    */
}