#include <QApplication>
#include "ReadImageQt.h"
#include "WriteImage.h"
#include <opencv2/opencv.hpp>
#include "utils.h"
#include <opencv2/imgproc.hpp>

int main (int argc, char *argv[])
{
    // READING
    /*
    QApplication app(argc, argv);
    ReadImageQt readImageQt;
    readImageQt.showImage("images/raw/lion.jpg");
    readImageQt.show();
    return app.exec();
    */
   
    //PROCESSING
    cv::Mat image = cv::imread("images/raw/lion.jpg");

    if(image.empty())
    std::cerr << "Image could not be loaded" << "\n";

    image = fitImageToWindow(image, 1280, 720);

    //stretchColorChannels(image, 0, 255);

    // transformLogarithmic(image, 0.2, 255);

    // transformHistEqual(image, 2.0, cv::Size (8,8), "local");

    transformBGRToHSI(image, 255.0);

    cv::imshow("Output", image);
    cv::waitKey(0);


    //WRITING
    /*
    WriteImage writer;
    bool success = writer.saveImage(image, "images/mod/path.jpg");
    return 0;
    */
}