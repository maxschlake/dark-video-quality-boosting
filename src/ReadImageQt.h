#ifndef READ_IMAGE_QT
#define READ_IMAGE_QT

#include <QWidget>
#include <QVBoxLayout>
#include <QStatusBar>
#include "ImageLabel.h"

class ReadImageQt : public QWidget
{
    Q_OBJECT

public:
    ReadImageQt(QWidget *parent = nullptr);
    void showImage(const QString &imagePath, double scaleFactor = 1.0);

private:
    ImageLabel *imageLabel;
    QStatusBar *statusBar;
    QVBoxLayout *layout;

private slots:
    void updateStatusBar(int r, int g, int b);
};

#endif 