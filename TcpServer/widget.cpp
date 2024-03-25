#include "widget.h"
#include "ui_widget.h"
#include <QFile>
#include <QMessageBox>
#include <QDebug>
#include "mytcpserver.h"
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    // 加载配置文件
    loadingConfig();

    // 监听
    MyTcpServer::getInstance().listen(QHostAddress(m_strIP),m_usPort);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::loadingConfig()
{
    QFile configFile(":/server.config");
    if(configFile.open(QIODevice::ReadOnly))
    {
        QByteArray info = configFile.readAll();
        configFile.close();
        QString infoStr = info.toStdString().c_str();
        infoStr.replace("\r\n"," ");
        QStringList infoList = infoStr.split(" ");

        m_strIP = infoList.at(0);
        m_usPort = infoList.at(1).toUInt();
        qDebug() << m_strIP << " " << m_usPort;

    }
    else{
        QMessageBox::critical(this,"config open","config open err");
    }
}