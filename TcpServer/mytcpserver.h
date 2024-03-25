#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QTcpServer>
#include "mytcpsocket.h"
#include <QList>
class MyTcpServer : public QTcpServer
{
    Q_OBJECT
public:
    MyTcpServer();
    /**
     * @brief getInstance
     * @return  单例设计模式 饿汉式 返回QTcpServer的实例
     */
    static MyTcpServer &getInstance();

    // 当有新的连接可用时，QTcpServer调用这个虚函数。socketDescriptor参数是已接受连接的本机套接字描述符。
    void incomingConnection(qintptr socketDescriptor);

private:
    QList<MyTcpSocket*> m_tcpSocketList;
};

#endif // MYTCPSERVER_H
