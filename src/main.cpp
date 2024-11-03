#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <QApplication>
#include "utils.h"
#include "processor.h"
#include "ReadImageQt.h"

int main (int argc, char *argv[])
{
    /*
    if (argc < 8)
    {
        std::cerr << "Usage: " << argv[0] << "\n"
        << "<image|video>" << "\n"
        << "<path_to_file>" << "\n"
        << "<output_file_name>" << "\n"
        << "<file_type>" << "\n"
        << "<L>" << "\n"
        << "<verbose>" << "\n"
        << "<transform_type>" << "\n"
        << "[<input_scale>]" << "\n"
        << "[<clip_limit>]" << "\n"
        << "[<tile_grid_width>]" << "\n"
        << "[<tile_grid_height>]" << "\n";
        return -1;
    }

    // Command line argument parsing
    const std::string mode = argv[1];                               // "image" or "video"
    const std::string rawFilePath = argv[2];                        // Path to image or video
    const std::string fileName = argv[3];                           // Output file name
    const std::string fileType = argv[4];                           // Output file type
    const int L = std::stoi(argv[5]);                               // Number of unique color intensities
    const bool verbose = std::stoi(argv[6]) != 0;                   // Verbose flag
    const std::string transformType = argv[7];                      // Transform type

    // Initialize optional parameters with defaults
    double inputScale = 1.0;
    double clipLimit = 0.0;
    cv::Size tileGridSize(8, 8);

    // Handle optional parameters based on the transformation type
    if (transformType == "locHE" || transformType == "globHE")
    {
        if (argc < 11)
        {
            std::cerr << "Error: For " << transformType << ", specify <clip_limit>, <tile_grid_width>, <tile_grid_height>." << "\n";
            return -1;
        }
        clipLimit = std::stod(argv[9]);
        tileGridSize = cv::Size(std::stoi(argv[10]), std::stoi(argv[11]));
    }
    else if (transformType == "log")
    {
        if (argc < 9)
        {
            std::cerr << "Error: For log transformation, specify <input_scale>." << "\n";
            return -1;
        }
        inputScale = std::stod(argv[8]);
    }
    
    // Process based on mode
    if (mode == "image")
    {
        const std::string filePath = fileName + "." + fileType;
        const std::string rawImageDir = "images/raw/";
        const std::string modImageDir = "images/mod/";
        const std::string histPath = "images/hist/";
        const std::string rawImagePath = rawImageDir + rawFilePath;
        const std::string modImagePath = modImageDir + fileName + "." + fileType;

        int result = processImage(
            rawImagePath, modImagePath, fileName, filePath, histPath, transformType, L, verbose, inputScale, clipLimit, tileGridSize);
        return result;
    }
    else if (mode == "video")
    {
        ;
    }
    else
    {
        std::cerr << "Error: Unknown mode: " << mode << "\n";
    }
    */
    
    

    const std::string mode = "video";
    const std::string fileName = "v1_short";
    const std::string fileType = "mp4";
    const std::string transformType = "locHE";
    const bool show = true;
    const int L = 256;
    const bool verbose = true;
    const double inputScale = 0.2;
    const double clipLimit = 40.0;
    const cv::Size tileGridSize(8, 8);

    const std::string file = fileName + "." + fileType;

    const std::string rawImageDir = "images/raw/";
    const std::string modImageDir = "images/mod/";
    const std::string histDir = "images/hist/";
    const std::string rawImagePath = rawImageDir + file;
    const std::string modImageFilePath = modImageDir + fileName + "_" + transformType + ".jpg";

    const std::string rawVideoDir = "videos/raw/";
    const std::string modVideoDir = "videos/mod/";
    const std::string rawVideoPath = rawVideoDir + file;
    const std::string modVideoFilePath = modVideoDir + fileName + "_" + transformType + ".mp4";

    std::cout << "HERE" << "\n";

    if (mode == "image")
    {
        processImage(
            rawImagePath, fileName, file, modImageFilePath, histDir, mode, transformType, L, verbose, inputScale, clipLimit, tileGridSize);
    }
    else if (mode == "video")
    {
        processVideo(
            rawVideoPath, fileName, modVideoFilePath, mode, transformType, L, verbose, inputScale, clipLimit, tileGridSize);
    }
    else
    {
        std::cerr << "Error: Unknown mode: " << mode << "\n";
    }

    if (mode == "image" && show)
    {
        QApplication app(argc, argv);
        ReadImageQt readImageQt;
        readImageQt.showImage(QString::fromStdString(modImageFilePath));
        readImageQt.show();
        return app.exec();
    }
    return 0;
}