#include "privatechat.h"
#include "ui_privatechat.h"

PrivateChat::PrivateChat(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PrivateChat)
{
    ui->setupUi(this);
}

PrivateChat::~PrivateChat()
{
    delete ui;
}

PrivateChat &PrivateChat::getInstance()
{
    static PrivateChat instance;
    return instance;
}

void PrivateChat::setMyName(QString name)
{
    this->myName = name;
}

void PrivateChat::setFriendName(QString name)
{
    this->friendName = name;
}

void PrivateChat::appendShowMsgTextEdit(QString msg)
{
    ui->showMsg_te->append(msg);
}

void PrivateChat::clearShowMsgTextEdit()
{
    ui->showMsg_te->clear();
}

void PrivateChat::on_sendMsg_pb_clicked()
{
    QString inputMsg = ui->inputMsg_le->text();
    if(inputMsg.isEmpty())
    {
        return;
    }
    emit sendInputMsgSignal(myName,friendName,inputMsg);
    ui->inputMsg_le->clear();
    QString msg = myName + "[myself] said:" + inputMsg;
    ui->showMsg_te->append(msg);
}

