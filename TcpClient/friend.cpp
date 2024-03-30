#include "friend.h"

#include "widget.h"
#include <QInputDialog>
#include <QMessageBox>
#include <string.h>
#include "privatechat.h"
#include "protocol.h"
Friend::Friend(QWidget *parent)
    : QWidget{parent}
{
    m_pFriendListWidget = new QListWidget;
    m_pShowMsgTE = new QTextEdit;
    m_pInputMsgLE = new QLineEdit;

    m_pShowMsgTE->setReadOnly(true);

    m_pMsgSendPB = new QPushButton("信息发送");
    m_pDelFriendPB = new QPushButton("删除好友");
    m_pFlushFriendPB = new QPushButton("刷新好友");
    m_pShowOnlineUsrPB = new QPushButton("显示在线用户");
    m_pSearchUsrPB = new QPushButton("查找用户");
    m_pPrivateChatPB = new QPushButton("私聊");

    m_online = new Online;

    QVBoxLayout *pRightBtnLayout = new QVBoxLayout;
    pRightBtnLayout->addWidget(m_pDelFriendPB);
    pRightBtnLayout->addWidget(m_pFlushFriendPB);
    pRightBtnLayout->addWidget(m_pShowOnlineUsrPB);
    pRightBtnLayout->addWidget(m_pSearchUsrPB);
    pRightBtnLayout->addWidget(m_pPrivateChatPB);

    QHBoxLayout *pBottomLayout = new QHBoxLayout;
    pBottomLayout->addWidget(m_pInputMsgLE);
    pBottomLayout->addWidget(m_pMsgSendPB);

    QHBoxLayout *pTopLayout = new QHBoxLayout;
    pTopLayout->addWidget(m_pShowMsgTE);
    pTopLayout->addWidget(m_pFriendListWidget);
    pTopLayout->addLayout(pRightBtnLayout);

    QVBoxLayout *pMain = new QVBoxLayout;
    pMain->addLayout(pTopLayout);
    pMain->addLayout(pBottomLayout);
    pMain->addWidget(m_online);
    m_online->hide();




    setLayout(pMain);

    connect(m_pShowOnlineUsrPB,SIGNAL(clicked(bool))
            ,this,SLOT(showHideOnlineWidget()));

    connect(m_pSearchUsrPB,SIGNAL(clicked(bool))
            ,this,SLOT(searchUsrInfoByName()));

    connect(m_online,SIGNAL(addFriendBtnClicked(QString))
            ,this,SLOT(handleAddFriendClicked(QString)));

    connect(m_pFlushFriendPB,SIGNAL(clicked(bool))
            ,this,SLOT(flushFriend()));

    connect(m_pDelFriendPB,SIGNAL(clicked(bool))
            ,this,SLOT(delFriend()));

    connect(m_pPrivateChatPB,SIGNAL(clicked(bool))
            ,this,SLOT(showPrivateChatWidget()));

    connect(&PrivateChat::getInstance(),SIGNAL(sendInputMsgSignal(QString,QString,QString))
            ,this,SLOT(sendPrivateChatInputMsg(QString,QString,QString)));

    connect(m_pMsgSendPB,SIGNAL(clicked(bool))
            ,this,SLOT(sendGroupChatMsg()));
}

Friend::~Friend()
{
    qDebug() << "friend delete";
    m_pFriendListWidget->clear();
    delete m_pFriendListWidget;
    m_pFriendListWidget = NULL;
    delete m_pShowMsgTE;
    m_pShowMsgTE = NULL;
    delete m_pInputMsgLE;
    m_pInputMsgLE = NULL;
    delete m_pShowMsgTE;
    m_pShowMsgTE = NULL;
    delete m_pMsgSendPB;
    m_pMsgSendPB = NULL;
    delete m_pDelFriendPB;
    m_pDelFriendPB = NULL;
    delete m_pFlushFriendPB;
    m_pFlushFriendPB = NULL;
    delete m_pShowOnlineUsrPB;
    m_pShowOnlineUsrPB = NULL;
    delete m_pSearchUsrPB;
    m_pSearchUsrPB = NULL;
    delete m_pPrivateChatPB;
    m_pPrivateChatPB = NULL;
    delete m_online;
    m_online = NULL;
}

void Friend::showAllOnline(QStringList nameList)
{
    m_online->setOnlineAllListWidget(nameList);
}

void Friend::showOnlineFriend(QStringList nameList)
{
    m_pFriendListWidget->clear();
    m_pFriendListWidget->addItems(nameList);
    m_friends = nameList;
}

void Friend::showRecvGroupChatMsg(QString msg)
{
    m_pShowMsgTE->append(msg);
}

QStringList Friend::getOnlineFriends()
{
    if(m_friends.isEmpty())
        flushFriend();
    return m_friends;

}

void Friend::showHideOnlineWidget()
{
    if(m_online->isHidden())
    {
        m_online->setHidden(false);
        m_pShowOnlineUsrPB->setText("隐藏在线用户");
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_REQUEST;
        Widget::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
    else
    {
        m_online->setHidden(true);
        m_pShowOnlineUsrPB->setText("显示在线用户");
    }
}

void Friend::searchUsrInfoByName()
{
    QString name = QInputDialog::getText(this,"搜索","用户名：");
    if(name.isEmpty())
    {

        return;
    }

    m_currentSearchName = name;

    PDU *pdu = mkPDU(0);
    pdu->uiMsgType = ENUM_MSG_TYPE_SEARCH_REQUEST;
    memcpy(pdu->caData,name.toStdString().c_str(),32);

    qDebug() << pdu->caData;

    Widget::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Friend::handleAddFriendClicked(QString friendName)
{
    QString myName = Widget::getInstance().getUsrName();
    if(friendName == myName)
    {
        QMessageBox::information(&Widget::getInstance(),"警告","不可添加自己");
        return;
    }

    PDU* pdu = mkPDU(0);
    pdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REQUEST;
    memcpy(pdu->caData,friendName.toStdString().c_str(),32);
    memcpy(pdu->caData+32,myName.toStdString().c_str(),32);

    Widget::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    // qDebug() << myName << " " << friendName;
    free(pdu);
    pdu = NULL;
}

void Friend::flushFriend()
{
    QString name = Widget::getInstance().getUsrName();  // 获取登录用户名
    PDU* pdu = mkPDU(0);
    memcpy(pdu->caData,name.toStdString().c_str(),name.size());
    pdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST;
    Widget::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Friend::delFriend()
{
    QString friendName = m_pFriendListWidget->currentItem()->text();
    if(friendName.isEmpty())
    {
        return;
    }
    QString myName = Widget::getInstance().getUsrName();    // 获取登录用户名
    PDU *pdu = mkPDU(0);
    memcpy(pdu->caData,friendName.toStdString().c_str(),friendName.size());
    memcpy(pdu->caData+32,myName.toStdString().c_str(),myName.size());
    pdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST;
    Widget::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Friend::showPrivateChatWidget()
{
    PrivateChat::getInstance().clearShowMsgTextEdit();
    QString friendName = m_pFriendListWidget->currentItem()->text();
    if(friendName.isEmpty())
        return;
    if(PrivateChat::getInstance().isHidden())
        PrivateChat::getInstance().show();

    PrivateChat::getInstance().setMyName(Widget::getInstance().getUsrName());
    PrivateChat::getInstance().setFriendName(friendName);
}

void Friend::sendPrivateChatInputMsg(QString myName, QString friendName, QString inputMsg)
{
    PDU *pdu = mkPDU(inputMsg.size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST;

    memcpy(pdu->caData
           ,myName.toStdString().c_str()
           ,myName.size());
    memcpy(pdu->caData+32
           ,friendName.toStdString().c_str()
           ,friendName.size());
    memcpy((char*)(pdu->caMsg)
           ,inputMsg.toStdString().c_str()
           ,inputMsg.size());
    Widget::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu = NULL;

}

void Friend::sendGroupChatMsg()
{
    QString msg = m_pInputMsgLE->text();
    if(msg.isEmpty())
    {
        QMessageBox::warning(this,"警告","输入不能为空");
        return ;
    }
    QString myName = Widget::getInstance().getUsrName();
    PDU* pdu = mkPDU(msg.size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_GROUP_CHAT_REQUEST;
    memcpy(pdu->caData,myName.toStdString().c_str(),myName.size());
    memcpy((char*)pdu->caMsg,msg.toStdString().c_str(),msg.size());
    Widget::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);

    QString showMsg = myName + "[myself] said: " + msg;
    m_pShowMsgTE->append(showMsg);
    m_pInputMsgLE->clear();
    free(pdu);
    pdu = NULL;
}
