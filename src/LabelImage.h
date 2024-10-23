#ifndef LABEL_IMAGE_H
#define LABEL_IMAGE_H

#include <QLabel>
#include <QMouseEvent>

class LabelImage : public QLabel
{
    Q_OBJECT

public:
    explicit LabelImage(QWidget *parent = nullptr); // Constructor

signals:
    void rgbValueChanged(int red, int green, int blue); // Signal to emit RGB values

protected: 
    void mouseMoveEvent(QMouseEvent * parent) override; // Override mouse move event to get RGB

};

#endif