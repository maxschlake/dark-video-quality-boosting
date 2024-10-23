#include "ImageLabel.h"
#include <QMouseEvent>
#include <QPixmap>
#include <QImage>

ImageLabel::ImageLabel(QWidget *parent) : QLabel(parent)
{
    setMouseTracking(true);
    setAlignment(Qt::AlignCenter);
    setScaledContents(true);
    setText("No image loaded.");
}

void ImageLabel::mouseMoveEvent(QMouseEvent *event)
{
    if (pixmap().isNull())
        return;
    
    // Get image from the QLabel
    QImage image = pixmap().toImage();

    // Get the coordinates of the mouse pointer
    int x = event->pos().x();
    int y = event->pos().y();

    // Ensure coordinates are within the image bounds
    if (x >= 0 && x < image.width() && y >= 0 && y < image.height())
    {
        // Get the pixel value (QRgb) at the mouse coordinates
        QRgb pixel = image.pixel(x, y);

        // Extract the RGB values
        int red = qRed(pixel);
        int green = qGreen(pixel);
        int blue = qBlue(pixel);

        // Emit the signal with RGB values
        emit rgbValueChanged(red, green, blue);
    }
}