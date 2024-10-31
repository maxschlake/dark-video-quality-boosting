#include <QApplication>
#include "ReadImageQt.h"
#include "WriteImage.h"
#include <opencv2/opencv.hpp>
#include "utils.h"
#include <opencv2/imgproc.hpp>

int main (int argc, char *argv[])
{
    // READING

    std::string fileName = "path";
    std::string fileType = "jpg";
    std::string filePath = fileName + "." + fileType;
    std::string rawPath = "images/raw/";
    std::string modPath = "images/mod/";
    std::string histPath = "images/hist/";
    std::string rawImagePath = rawPath + filePath;
    std::string modImagePath = modPath + filePath;

    /*
    QApplication app(argc, argv);
    ReadImageQt readImageQt;
    readImageQt.showImage(rawImagePath);
    readImageQt.show();
    return app.exec();
    */
   
    //PROCESSING
    cv::Mat image = cv::imread(rawImagePath);

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
    std::map<double, int> originalHSIHist = computeChannelHist(hsiImage, channelIndex, L, cMax);

    double clippingLimit = computeClippingLimit(originalHSIHist, L);

    int M;
    std::map<double, double> clippedHSIHist = computeClippedChannelHist(originalHSIHist, clippingLimit, M);

    double pmax, pmin;
    std::map<double, double> PDF = computePDF(clippedHSIHist, M, pmax, pmin);

    std::map<double, double> CDF = computeCDF(PDF);

    double WHDFSum;
    std::map<double, double> WHDF = computeWHDF(PDF, CDF, WHDFSum, pmax, pmin, cMax);

    std::map<double, double> gamma = computeGamma(WHDF, WHDFSum, cMax);

    cv::Mat transformedHSIImage = transformChannel(hsiImage, channelIndex, gamma, cMax);

    std::map<double, int> transformedHSIHist = computeChannelHist(transformedHSIImage, channelIndex, L, cMax);
    
    int yMax, yMid;
    plotHistogram(transformedHSIHist, L, fileName, histPath, filePath, yMax, yMid, 40, true, false);
    plotHistogram(originalHSIHist, L, fileName, histPath, filePath, yMax, yMid, 40, false, false);

    cv::Mat bgrImage = transformHSIToBGR(transformedHSIImage, L - 1, "BGR");

    //return 0;

    //cv::imshow("Output", image);
    //cv::waitKey(0);
    
    WriteImage writer;
    bool success = writer.saveImage(bgrImage, modImagePath);
    QApplication app(argc, argv);
    ReadImageQt readImageQt;
    readImageQt.showImage(QString::fromStdString(modImagePath));
    readImageQt.show();
    return app.exec();
    


    //WRITING
    /*
    WriteImage writer;
    bool success = writer.saveImage(image, "images/mod/path.jpg");
    return 0;
    */
}