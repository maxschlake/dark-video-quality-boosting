#include "ReadImageQt.h"
#include <QPixmap>
#include <QImage>
#include <opencv2/opencv.hpp>

ReadImageQt::ReadImageQt(QWidget *parent) : QWidget(parent)
{
    setWindowTitle("Image RGB viewer");

    layout = new QVBoxLayout(this);
    labelImage = new LabelImage(this);
    labelImage->setMouseTracking(true);

    statusBar = new QStatusBar(this);
    statusBar->showMessage("Move your mouse over the image to see the RGB values");

    connect(labelImage, &LabelImage::rgbValueChanged, this, &ReadImageQt::updateStatusBar);

    layout->addWidget(labelImage);
    layout->addWidget(statusBar);
}

void ReadImageQt::showImage(const QString &imagePath, double scaleFactor)
{
    cv::Mat image = cv::imread(imagePath.toStdString());
    if (!image.empty())
    {
        // Optionally, scale the image with the provided scaling factor 
        if (scaleFactor != 1.0)
        {
            cv::Size newSize(static_cast<int>(image.cols * scaleFactor), static_cast<int>(image.rows * scaleFactor));
            cv::resize(image, image, newSize);
        }
        
        // Convert from BGR to RGB for accurate display in Qt
        cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
        
        // Convert the image to QPixmap
        QImage qImage(image.data, image.cols, image.rows, image.step[0], QImage::Format_RGB888);
        labelImage->setPixmap(QPixmap::fromImage(qImage));
        labelImage->resize(qImage.size());
    }
    else
    {
        statusBar->showMessage("Error: Could not load image.");
    }
}

void ReadImageQt::updateStatusBar(int r, int g, int b)
{
    statusBar->showMessage(QString("R: %1, G: %2, B: %3").arg(r).arg(g).arg(b));
}