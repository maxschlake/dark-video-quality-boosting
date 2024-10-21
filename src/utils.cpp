#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <filesystem>
#include <QApplication>
#include <QWizard>
#include <QLabel>
#include <QPixmap>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QStatusBar>

// Function to show an image using QWidget

// Subclass QLabel to handle mouse events
class ImageLabel : public QLabel
{
    Q_OBJECT

public: 
    ImageLabel(QWidget *parent = nullptr) : QLabel(parent) {}

protected:
    void mouseMoveEvent(QMouseEvent * event) override
    {
        if (pixmap().isNull())
            return;
        
        // Get image from the QLabel
        QImage image = pixmap().toImage();

        // Get the coordinates of the mouse pointer
        int x = event -> pos().x();
        int y = event -> pos().y();

        // Ensure coordinates are within the image bounds
        if (x >= 0 && x < image.width() && y >= 0 && y < image.height())
        {
            // Get the pixel value (QRgb) at the mouse coordinates
            QRgb pixel = image.pixel(x, y);

            // Extract the RGB values 
            int red = qRed(pixel);
            int green = qGreen(pixel);
            int blue = qBlue(pixel);

            // Update the status bar with the RGB values
            emit rgbValueChanged(red, green, blue);
        }
    }

signals:
    void rgbValueChanged(int red, int green, int blue);
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Main window widget
    QWidget window;
    window.setWindowTitle("Image RGB viewer");
    window.resize(800, 600);

    // Layout to organize the image and status bar
    QVBoxLayout *layout = new QVBoxLayout(&window);

    // Create custom label to display the image
    ImageLabel *imageLabel = new ImageLabel(&window);
    imageLabel->setMouseTracking(true);

    // Load the image using QPixmap
    QPixmap pixmap("images/penguin.jpg");
    imageLabel->setPixmap(pixmap);
    imageLabel->resize(pixmap.size());

    // Status bar to display the RGB values 
    QStatusBar *statusBar = new QStatusBar(&window);
    statusBar->showMessage("Move your mouse over the image to see the RGB values");

    // Connect the signal to update the RGB values in the status bar
    QObject::connect(imageLabel, &ImageLabel::rgbValueChanged, [&](int r, int g, int b)
    {
        statusBar->showMessage(QString("R: %1, G: %2, B: %3").arg(r).arg(g).arg(b));
    });

    // Add the image and status bar to the layout
    layout->addWidget(imageLabel);
    layout->addWidget(statusBar);

    // Show the main window
    window.show();

    return app.exec();

}

#include "utils.moc"

/*
int main()
{
    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_WARNING);
    std::string imagePath = "images/penguin.jpg";
    std::cout << "Current path: " << std::filesystem::current_path() << "\n";
    cv::Mat image = cv::imread(imagePath, cv::IMREAD_COLOR);
    cv::imshow("Display window", image);
    cv::waitKey(0);
    return 0;
}
*/

/*
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QWidget w;
    w.show();
    return a.exec();
}  
*/ 

