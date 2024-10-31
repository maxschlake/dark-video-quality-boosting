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
    cv::Mat image = cv::imread("images/raw/path.jpg");

    if(image.empty())
    std::cerr << "Image could not be loaded" << "\n";

    image = fitImageToWindow(image, 1280, 720);

    const double L = 256.0;
    //stretchColorChannels(image, 0, L - 1);

    //transformLogarithmic(image, 0.2, L - 1);

    //transformHistEqual(image, 2.0, cv::Size (8,8), "local");
    
    std::cout << "Image Size: " << image.size() << "\n";
    cv::Mat hsiImage = transformBGRToHSI(image, L - 1, "BGR");
    std::cout << "HSI Image Size: " << hsiImage.size() << "\n";

    int channelIndex = 0;
    double cMax;
    std::map<double, int> origHist = computeChannelHist(hsiImage, channelIndex, L, cMax);

    //plotHistogram(origHist);

    double clippingLimit = computeClippingLimit(origHist, L);

    int M;
    std::map<double, double> clippedHist = computeClippedChannelHist(origHist, clippingLimit, M);

    //plotHistogram(clippedHist);

    double pmax, pmin;
    std::map<double, double> PDF = computePDF(clippedHist, M, pmax, pmin);

    std::map<double, double> CDF = computeCDF(PDF);

    double WHDFSum;
    std::map<double, double> WHDF = computeWHDF(PDF, CDF, WHDFSum, pmax, pmin, cMax);

    std::map<double, double> gamma = computeGamma(WHDF, WHDFSum, cMax);

    cv::Mat transformedHSIImage = transformChannel(hsiImage, channelIndex, gamma, cMax);

    std::cout << origHist.size() << ", " << clippedHist.size() << "\n";

    cv::Mat bgrImage = transformHSIToBGR(transformedHSIImage, L - 1, "BGR");

    //return 0;

    //cv::imshow("Output", image);
    //cv::waitKey(0);
    
    WriteImage writer;
    bool success = writer.saveImage(bgrImage, "images/mod/path.jpg");
    QApplication app(argc, argv);
    ReadImageQt readImageQt;
    readImageQt.showImage("images/mod/path.jpg");
    readImageQt.show();
    return app.exec();
    


    //WRITING
    /*
    WriteImage writer;
    bool success = writer.saveImage(image, "images/mod/path.jpg");
    return 0;
    */
}