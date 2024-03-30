#include "mytcpserver.h"
#include <QDebug>
MyTcpServer::MyTcpServer() {}

MyTcpServer &MyTcpServer::getInstance()
{
    static MyTcpServer instance;
    return instance;
}

void MyTcpServer::incomingConnection(qintptr socketDescriptor)
{
    qDebug() << "new client connected";

    MyTcpSocket *pTcpSocket = new MyTcpSocket;
    pTcpSocket->setSocketDescriptor(socketDescriptor);
    m_tcpSocketList.append(pTcpSocket);

    connect(pTcpSocket,SIGNAL(offline(MyTcpSocket*))
            ,this,SLOT(deleteSocket(MyTcpSocket*)));

    connect(pTcpSocket,SIGNAL(addFriendRelaySignal(QString,PDU*))
            ,this,SLOT(handleRelay(QString,PDU*)));

    connect(pTcpSocket,SIGNAL(delFriendRelaySignal(QString,PDU*))
            ,this,SLOT(handleRelay(QString,PDU*)));

    connect(pTcpSocket,SIGNAL(privateChatRelaySignal(QString,PDU*))
            ,this,SLOT(handleRelay(QString,PDU*)));

    connect(pTcpSocket,SIGNAL(groupChatRelaySignal(QString,PDU*))
            ,this,SLOT(handleRelay(QString,PDU*)));

    connect(pTcpSocket,SIGNAL(shareFileRelaySignal(QString,PDU*))
            ,this,SLOT(handleRelay(QString,PDU*)));
}

void MyTcpServer::deleteSocket(MyTcpSocket *socket)
{
    QList<MyTcpSocket*>::iterator iter;
    for(iter = m_tcpSocketList.begin();iter!=m_tcpSocketList.end();iter++)
    {
        if(socket == *iter)
        {
            delete *iter;
            *iter = nullptr;
            m_tcpSocketList.erase(iter);
            break;
        }
    }

    for(int i=0;i<m_tcpSocketList.length();i++)
    {
        qDebug() << m_tcpSocketList.at(i)->getName();
    }
}

void MyTcpServer::handleRelay(QString name, PDU *pdu)
{
    for(int i=0;i<m_tcpSocketList.length();i++)
    {
        if(m_tcpSocketList.at(i)->getName() == name)
        {
            m_tcpSocketList.at(i)->write((char*)pdu,pdu->uiPDULen);
            break;
        }
    }
}
