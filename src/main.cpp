#include <QApplication>
#include "ReadImageQt.h"
#include "WriteImage.h"
#include <opencv2/opencv.hpp>
#include "utils.h"

int main (int argc, char *argv[])
{
    // READING
    /*
    QApplication app(argc, argv);
    ReadImageQt readImageQt;
    readImageQt.showImage("images/raw/penguin.jpg");
    readImageQt.show();
    return app.exec();
    */
   
    //PROCESSING
    cv::Mat image = cv::imread("images/raw/jam.jpg");

    fitImageToWindow(image, 800, 600);

    stretchColorChannels(image, 0, 255);

    cv::imshow("stretched", image);
    cv::waitKey(0);


    //WRITING
    /*
    WriteImage writer;
    bool success = writer.saveImage(image, "images/mod/path.jpg");
    return 0;
    */
}