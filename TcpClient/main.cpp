#include "widget.h"
#include <QApplication>
// #include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget::getInstance().show();


    return a.exec();
}
