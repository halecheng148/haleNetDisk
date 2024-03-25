#include "mytcpsocket.h"
#include <QDebug>
#include "protocol.h"
MyTcpSocket::MyTcpSocket(QObject *parent)
    : QTcpSocket{parent}
{
    connect(this,SIGNAL(readyRead())
            ,this,SLOT(recvMsg()));
}

void MyTcpSocket::recvMsg()
{
    uint uiPDULen = 0;
    this->read((char*)&uiPDULen,sizeof(uint));

    uint uiMsgLen = uiPDULen-sizeof(PDU);

    PDU *pdu = mkPDU(uiMsgLen);

    this->read((char*)pdu+sizeof(uint),uiPDULen-sizeof(uint));

    qDebug() << pdu->uiMsgType << (char*)(pdu->caMsg);
}
