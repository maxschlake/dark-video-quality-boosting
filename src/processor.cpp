#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <filesystem>
#include "utils.h"
#include "processor.h"

void processImage(
    const std::string& rawImagePath, const std::string& modImagePath, const std::string& fileName, const std::string& mode,
    const std::string& filePath, const std::string& histPath, const std::string transformType, int L, bool verbose, double inputScale, 
    double clipLimit, const cv::Size tileGridSize)
{
    cv::Mat image = cv::imread(rawImagePath);

    if(image.empty())
    {
        std::cerr << "Error: Image file could not be opened." << "\n";
        return;
    }

    image = fitImageToWindow(image, 1280, 720);
    stretchColorChannels(image, 0, L);

    if (transformType == "log")
    {
        transformLogarithmic(image, inputScale, L);
    }
    else if (transformType == "locHE")
    {
        transformHistEqual(image, clipLimit, tileGridSize, "local");
    }
    else if (transformType == "globHE")
    {
        transformHistEqual(image, clipLimit, tileGridSize, "global");
    }
    else if (transformType == "AGCWHD")
    {
        transformAGCWHD(image, L, fileName, mode, verbose, histPath, filePath);
    }

    saveImage(image, modImagePath, verbose);
}

void processVideo(
    const std::string& rawVideoPath, const std::string& modVideoDir, const std::string& fileName, const std::string& mode,
    const std::string& transformType, int L, bool verbose, double inputScale, double clipLimit, const cv::Size tileGridSize)
{   
    cv::VideoCapture cap(rawVideoPath);
    if (!cap.isOpened())
    {
        std::cerr << "Error: Video file could not be opened." << "\n";
        return;
    }

    // Get video properties for the writer
    int frameWidth = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    int frameHeight = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    int fps = static_cast<int>(cap.get(cv::CAP_PROP_FPS));

    if (verbose)
    {
        std::cout << "frameWidth: " << frameWidth << ", frameHeight: " << frameHeight << ", fps: " << fps << "\n";
    }

    // Deactivate verbose for the single images
    bool verboseCopy = verbose;
    verbose = false;

    // Set up the output video writer and mod video directory
    std::string modVideoFilePath = modVideoDir + fileName + "_mod.mp4";
    const std::filesystem::path dirPath = std::filesystem::path(modVideoFilePath).parent_path();

    if (!std::filesystem::exists(dirPath))
    {
        std::filesystem::create_directories(dirPath);
    }
    cv::VideoWriter writer(modVideoFilePath, cv::VideoWriter::fourcc('m', 'p', '4', 'v'), fps, cv::Size(frameWidth, frameHeight));

    if (!writer.isOpened())
    {
        std::cerr << "Error: Video writer could not be opened." << "\n";
        return;
    }

    cv::Mat frame;
    int frameCount = 0;

    while (true)
    {
        cap >> frame;
        if (frame.empty())
        {
            break;
        }
            
        frame = fitImageToWindow(frame, 1280, 720);
        stretchColorChannels(frame, 0, L);

        if (transformType == "log")
        {
            transformLogarithmic(frame, inputScale, L);
        }
        else if (transformType == "locHE")
        {
            transformHistEqual(frame, clipLimit, tileGridSize, "local");
        }
        else if (transformType == "globHE")
        {
            transformHistEqual(frame, clipLimit, tileGridSize, "global");
        }
        else if (transformType == "AGCWHD")
        {
            transformAGCWHD(frame, L, fileName, mode, verbose);
        }

        // Write the processed frame to the output video file
        writer.write(frame);
        frameCount++;
    }

    // Release ressources
    cap.release();
    writer.release();

    // Reactivate verbose
    verbose = verboseCopy;
    if (verbose)
    {
        std::cout << "Processed video saved under: " << modVideoFilePath << "\n";
    }
}   