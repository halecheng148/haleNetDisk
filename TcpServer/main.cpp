#include "widget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    OpenDB::getInstance().init();
    Widget::getInstance().show();
    return a.exec();
}
