#ifndef FRIEND_H
#define FRIEND_H

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "online.h"

class Friend : public QWidget
{
    Q_OBJECT
public:
    explicit Friend(QWidget *parent = nullptr);
    ~Friend();

    void showAllOnline(QStringList nameList);

    /**
     * 刷新显示当前在线的所有好友到好友列表中
     * @brief showOnlineFriend
     * @param nameList
     */
    void showOnlineFriend(QStringList nameList);

    /**
     * 将接收的群聊信息，显示在m_pShowMsgTE中
     * @brief showRecvGroupChatMsg
     * @param msg
     */
    void showRecvGroupChatMsg(QString msg);

    QStringList getOnlineFriends();

signals:


public slots:
    /**
     * 显示或隐藏在线用户列表
     * @brief showHideOnlineWidget
     */
    void showHideOnlineWidget();

    /**
     * 当查询按键点击时，通过name查询用户
     * name:通过弹窗获取
     * @brief searchUsrInfoByName
     */
    void searchUsrInfoByName();


    void handleAddFriendClicked(QString friendName);

    /**
     * 刷新好友列表
     * @brief flushFriend
     */
    void flushFriend();


    /**
     * 删除好友按钮的槽函数
     * @brief delFriend
     */
    void delFriend();

    /**
     * 显示私聊窗口，并为窗口绑定双方信息
     * @brief showPrivateChatWidget
     */
    void showPrivateChatWidget();

    /**
     * 私聊消息发送
     * @brief sendPrivateChatInputMsg
     * @param myName
     * @param friendName
     * @param inputMsg
     */
    void sendPrivateChatInputMsg(QString myName,QString friendName,QString inputMsg);

    /**
     * 群聊消息发送
     * @brief sendGroupChatMsg
     */
    void sendGroupChatMsg();

private:
    QTextEdit *m_pShowMsgTE;                // 显示信息
    QListWidget *m_pFriendListWidget;       // 显示好友列表
    QLineEdit *m_pInputMsgLE;               // 信息输入框

    QPushButton *m_pDelFriendPB;            // 删除好友按钮
    QPushButton *m_pFlushFriendPB;          // 刷新在线好友列表按钮
    QPushButton *m_pShowOnlineUsrPB;        // 查看在线用户
    QPushButton *m_pSearchUsrPB;            // 查找用户
    QPushButton *m_pMsgSendPB;              // 发送信息按钮
    QPushButton *m_pPrivateChatPB;          // 私聊按钮
    Online *m_online;                       // 在线用户列表

    QStringList m_friends;

public:
    QString m_currentSearchName;            // 当前查找的用户名
};

#endif // FRIEND_H
