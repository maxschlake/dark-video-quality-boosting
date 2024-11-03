#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <filesystem>
#include "utils.h"
#include "processor.h"

void processImage(
    const std::string& rawImagePath, const std::string& fileName, const std::string& file, const std::string& modImageFilePath,
    const std::string& histDir, const std::string& mode, const std::string transformType, const int L, const bool verbose,
    const double inputScale, const double clipLimit, const cv::Size& tileGridSize)
{
    cv::Mat image = cv::imread(rawImagePath);
    if(image.empty())
    {
        std::cerr << "Error: Image file could not be opened." << "\n";
        return;
    }

    // Fit image to window and stretch the color channels
    image = fitImageToWindow(image, 1280, 720);
    stretchColorChannels(image, 0, L);

    // Perform image transformation depending on the chosen transform type
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
        transformAGCWHD(image, L, fileName, mode, verbose, histDir, file);
    }

    // Save the modified image
    saveImage(image, modImageFilePath, verbose);
}

void processVideo(
    const std::string& rawVideoPath, const std::string& fileName, const std::string& modVideoFilePath,
    const std::string& mode,const std::string& transformType, const int L, const bool verbose, 
    const double inputScale, const double clipLimit, const cv::Size& tileGridSize)
{   
    cv::VideoCapture cap(rawVideoPath);
    if (!cap.isOpened())
    {
        std::cerr << "Error: Video file could not be opened." << "\n";
        return;
    }
    
    // Set up mod video file path
    createDirectory(modVideoFilePath);

    // Get video properties for the writer
    int frameWidth = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    int frameHeight = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    int fps = static_cast<int>(cap.get(cv::CAP_PROP_FPS));

    if (verbose)
    {
        std::cout << "frameWidth: " << frameWidth << ", frameHeight: " << frameHeight << ", fps: " << fps << "\n";
    }

    // Set up the output video writer
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
            transformAGCWHD(frame, L, fileName, mode, false);
        }

        // Write the processed frame to the output video file
        writer.write(frame);
        frameCount++;
    }

    // Release ressources
    cap.release();
    writer.release();

    if (verbose)
    {
        std::cout << "Processed video saved under: " << modVideoFilePath << "\n";
    }
}   