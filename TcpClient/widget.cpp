#include "widget.h"
#include "ui_widget.h"
#include <QFile>
#include <QDebug>
#include <QMessageBox>
#include <QHostAddress>
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    LoadingConfig();

    // 连接成功弹窗
    connect(&m_socket,SIGNAL(connected()),this,SLOT(showConnected()));
    // 连接服务器,异步
    m_socket.connectToHost(QHostAddress(m_strIP),m_usPort);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::LoadingConfig()
{
    QFile configFile(":/client.config");
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

void Widget::showConnected()
{
    QMessageBox::information(this,"连接服务器","连接服务器成功");
}
