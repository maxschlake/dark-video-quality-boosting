#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <QApplication>
#include <filesystem>
#include "utils.h"
#include "processor.h"
#include "ReadImageQt.h"

void printUsage(const char* programName)
{
    std::cout << "\n" << "Usage: " << programName << "\n"
    << "For more information, see also: https://github.com/maxschlake/dark-video-quality-boosting" << "\n\n"
    << "<mode>                ----    <char>    Choose mode: 'image', 'video'\n"
    << "<rawFileDir>          ----    <char>    Enter directory of the raw file\n"
    << "<rawFileName>         ----    <char>    Enter name of the raw file\n"
    << "<rawFileType>         ----    <char>    Enter type of the raw file\n"
    << "<transformType>       ----    <char>    Choose transform type: 'log', 'locHE', 'globHE', 'AGCWHD'\n"
    << "<L>                   ----    <int>     Enter the number of possible intensity values\n"
    << "<verbose>             ----    <bool>    Show extended commentary: 'true', 'false'\n"
    << "[<show>]              ----    <bool>    Show output image (only for 'image' mode): 'true', 'false'\n"
    << "[<inputScale>]        ----    <double>  Enter the input scale (only for 'log' transform type)\n"
    << "[<clipLimit>]         ----    <double>  Enter the clip limit (only for 'locHE' transform type)\n"
    << "[<tileGridWidth>]     ----    <int>     Enter the tile grid width (only for 'locHE' transform type)\n"
    << "[<tileGridHeight>]    ----    <int>     Enter the tile grid height (only for 'locHE' transform type)\n";
}

int main (int argc, char *argv[])
{   
    // Set OpenCV logging level to ERROR to suppress warnings
    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_ERROR);

    // Check if no arguments are provided
    if (argc == 1)
    {
        std::cerr << "Error: No arguments provided. Please run the program with necessary arguments from the command line.\n";
        printUsage(argv[0]);
        std::cout << "\n" << "Press any key to exit." << "\n";
        std::cin.get();
        return -1;
    }

    // Check if the mandatory arguments are provided and do not start with '-'
    for (int i = 1; i <= 7; ++i) 
    {
        if (argv[i] == nullptr || argv[i][0] == '-') 
        {
            std::cerr << "Error: Argument " << i << " is required but not provided." << "\n";
            printUsage(argv[0]);
            return -1;
        }
    }

    // Command line argument parsing
    const std::string mode = argv[1];                               // mode: "image" or "video"
    const std::string rawFileDir = argv[2];                         // Directory of raw file
    const std::string rawFileName = argv[3];                        // Name of raw file
    const std::string rawFileType = argv[4];                        // Type of raw file
    const std::string transformType = argv[5];                      // Transform type: "log", "locHE", "globHE" or "AGCWHD"
    const int L = std::stoi(argv[6]);                               // Number of possible intensity values
    const bool verbose = (std::string(argv[7]) == "true");          // Show extended commentary

    // Initialize optional parameters with defaults
    bool show = false;                                              // Show output image (only for "image" mode)
    double inputScale = 1.0;                                        // input scale (only for logarithmic transformation)
    double clipLimit = 0.0;                                         // clip limit (only for local histogram equalization)
    cv::Size tileGridSize(8, 8);                                    // tile grid size (only for local histogram equalization)

    // Initialize optional parameter flags with defaults
    bool showProvided = false;
    bool inputScaleProvided = false;
    bool clipLimitProvided = false;
    bool tileGridWidthProvided = false;
    bool tileGridHeightProvided = false;

    // Parse named parameters
    for (int i = 8; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (arg == "--show" && mode == "image")
        {
            if (i + 1 < argc && argv[i + 1][0] != '-')
            {
                std::string showValue = argv[++i];
                show = (showValue == "true");
                showProvided = true;
            }
            else
            {
            std::cerr << "Error: '--show' requires 'true' or 'false'.\n";
            return -1;
            }
        }
        else if (arg == "--inputScale" && transformType == "log")
        {
            if (i + 1 < argc && argv[i + 1][0] != '-')
            {
                inputScale = std::stod(argv[++i]);
                inputScaleProvided = true;
            }
            else
            {
                std::cerr << "Error: '--inputScale' requires a value.\n";
                return -1;
            }
        }
        else if (arg == "--clipLimit" && transformType == "locHE")
        {
            if (i + 1 < argc && argv[i + 1][0] != '-')
            {
                clipLimit = std::stod(argv[++i]);
                clipLimitProvided = true;
            }
            else
            {
                std::cerr << "Error: '--clipLimit' requires a value.\n";
                return -1;
            }
        }
        else if (arg == "--tileGridWidth" && transformType == "locHE")
        {
            if (i + 1 < argc && argv[i + 1][0] != '-')
            {
                tileGridSize.width = std::stoi(argv[++i]);
                tileGridWidthProvided = true;
            }
            else
            {
                std::cerr << "Error: '--tileGridWidth' requires a value.\n";
                return -1;
            }
        }
        else if (arg == "--tileGridHeight" && transformType == "locHE")
        {
            if (i + 1 < argc && argv[i + 1][0] != '-')
            {
                tileGridSize.height = std::stoi(argv[++i]);
                tileGridHeightProvided = true;
            }
            else
            {
                std::cerr << "Error: '--tileGridHeight' requires a value.\n";
                return -1;
            }
        }
        else
        {
            std::cerr << "Unknown parameter: " << arg << "\n";
            return -1;
        }
    }

    // Throw errors if optional parameters are not provided for specific modes and transform types
    if (mode == "image" && !showProvided)
    {
        std::cerr << "Error: '--show' parameter is required for 'image' mode.\n";
        return -1;
    }
    if (transformType == "log" && !inputScaleProvided)
    {
        std::cerr << "Error: '--inputScale' parameter is required for 'log' transformation.\n";
        return -1;
    }
    if (transformType == "locHE" && !(clipLimitProvided && tileGridWidthProvided && tileGridHeightProvided))
    {
        std::cerr << "Error: '--clipLimit', '--tileGridWidth', and '--tileGridHeight' are required for 'locHE' transformation.\n";
        return -1;
    }

    // Create further directories and paths depending on the provided raw file path
    const std::string rawFile = rawFileName + "." + rawFileType;
    const std::string rawFilePath = rawFileDir + "/" + rawFile;
    const std::string modFileDir = std::filesystem::path(rawFileDir).parent_path().std::filesystem::path::string() + "/mod/";

    // Process the file according to the chosen mode
    if (mode == "image")
    {
        const std::string histDir = std::filesystem::path(rawFileDir).parent_path().std::filesystem::path::string() + "/hist/";
        const std::string modFilePath = modFileDir + rawFileName + "_" + transformType + ".jpg";

        processImage(
            rawFilePath, rawFileName, rawFile, modFilePath, histDir, mode, transformType, L, verbose, inputScale, clipLimit, tileGridSize);
        
        if (show)
        {
        QApplication app(argc, argv);
        ReadImageQt readImageQt;
        readImageQt.showImage(QString::fromStdString(modFilePath));
        readImageQt.show();
        return app.exec();
        }
    }
    else if (mode == "video")
    {
        const std::string modFilePath = modFileDir + rawFileName + "_" + transformType + ".mp4";

        processVideo(
            rawFilePath, rawFileName, modFilePath, mode, transformType, L, verbose, inputScale, clipLimit, tileGridSize);
    }
    else
    {
        std::cerr << "Error: Unknown mode: " << mode << "\n";
    }

    return 0;
}
