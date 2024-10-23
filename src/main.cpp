#include <QApplication>
#include "ReadImageQt.h"

int main (int argc, char *argv[])
{
    QApplication app(argc, argv);

    ReadImageQt readImageQt;
    readImageQt.showImage("images/penguin.jpg");
    readImageQt.show();

    return app.exec();
}