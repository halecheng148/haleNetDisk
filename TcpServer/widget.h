#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "mytcpserver.h"
QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    QString getUsrRootDirPath();

    static Widget &getInstance();

    void loadingConfig();
signals:
    void sendQuitSignal();
private:
    Ui::Widget *ui;

    QString m_strIP;
    quint16 m_usPort;
    QString m_rootDir;
};
#endif // WIDGET_H
