#include <QApplication>
#include "ReadImageQt.h"
#include "WriteImage.h"
#include <opencv2/opencv.hpp>

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

    //WRITING
    cv::Mat image = cv::imread("images/raw/penguin.jpg");
    WriteImage writer;
    bool success = writer.saveImage(image, "images/mod/penguin.jpg");
    return 0;
    
}