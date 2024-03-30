#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QTcpSocket>
#include <QFile>
#include <QTimer>
#include "opendb.h"
#include "protocol.h"
class MyTcpSocket : public QTcpSocket
{
    Q_OBJECT
public:
    explicit MyTcpSocket(QObject *parent = nullptr);

    QString getName();

public slots:
    /**
     * @brief recvMsg 读取数据
     */
    void recvMsg();
    /**
     * @brief clientOffline
     * 客户端结束时，服务器端对应的socket会自动发送disconnected()信号
     * 发送自定义信号offline()通知服务器，删除mytcpserver::m_socketlist中的对应socket
     */
    void clientOffline();

    /**
     * 开始向客户端传输文件数据
     * @brief downloadingFile
     */
    void downloadingFile();

    void copyDir(QString srcDir,QString destDir);


signals:
    void offline(MyTcpSocket *m_socket);

    /**
     * 向TcpServer发送的信号，目的是遍历其保存的在线socket转发添加好友请求，获取对方同意
     * @brief addFriendRelaySignal
     * @param pdu
     */
    void addFriendRelaySignal(QString name,PDU* pdu);

    /**
     * 转发被删除的通知，向name的用户
     * @brief delFriendRelaySignal
     * @param name
     * @param pdu
     */
    void delFriendRelaySignal(QString name,PDU* pdu);

    /**
     * 私聊转发
     * @brief privateChatRelaySignal
     * @param name
     * @param pdu
     */
    void privateChatRelaySignal(QString name,PDU* pdu);

    /**
     * 群聊转发
     * @brief groupChatRelaySignal
     * @param name
     * @param pdu
     */
    void groupChatRelaySignal(QString name,PDU* pdu);


    /**
     * 分享文件转发信号
     * @brief shareFileRelaySignal
     * @param name
     * @param pdu
     */
    void shareFileRelaySignal(QString name,PDU* pdu);


private:
    QString     strName;            // 记录用户名称
    QFile       m_file;             // 上传文件，打开的文件
    qint64      m_Total;            // 上传文件的总大小
    qint64      m_Recved;           // 以已经上传的文件大小
    bool        m_isUpload=false;   // 记录是否进入了接收上传文件的状态

    QTimer      m_timer;            // 用于下载文件的定时器
    QString     m_downloadFilePath;   // 要下载的文件路径
};

#endif // MYTCPSOCKET_H
