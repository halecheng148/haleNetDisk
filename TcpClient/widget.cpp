#include "widget.h"
#include "ui_widget.h"
#include <QFile>
#include <QDebug>
#include <QMessageBox>
#include <QHostAddress>

#include "protocol.h"
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

void Widget::on_pushButton_clicked()
{
    QString strMsg = ui->lineEdit->text();
    if(!strMsg.isEmpty())
    {
        PDU *pdu = mkPDU(strMsg.size()+1);
        // 设置消息类型
        pdu->uiMsgType = 1;
        // 封装消息
        strcpy((char*)pdu->caMsg,strMsg.toStdString().c_str());
        qDebug() << (char*)pdu->caMsg;
        // 发送消息
        m_socket.write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
    else
    {
        QMessageBox::information(this,"发送消息","发送失败");
    }
}

