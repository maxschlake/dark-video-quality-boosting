#ifndef IMAGE_LABEL_H
#define IMAGE_LABEL_H

#include <QLabel>
#include <QMouseEvent>

class ImageLabel : public QLabel
{
    Q_OBJECT

public:
    explicit ImageLabel(QWidget *parent = nullptr); // Constructor

signals:
    void rgbValueChanged(int red, int green, int blue); // Signal to emit RGB values

protected: 
    void mouseMoveEvent(QMouseEvent * parent) override; // Override mouse move event to get RGB

};

#endif