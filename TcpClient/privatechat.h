#ifndef PRIVATECHAT_H
#define PRIVATECHAT_H

#include <QWidget>

namespace Ui {
class PrivateChat;
}

class PrivateChat : public QWidget
{
    Q_OBJECT

public:
    explicit PrivateChat(QWidget *parent = nullptr);
    ~PrivateChat();

    static PrivateChat& getInstance();

    void setMyName(QString name);
    void setFriendName(QString name);

    void appendShowMsgTextEdit(QString msg);

    void clearShowMsgTextEdit();

signals:
    /**
     * 发送消息的信号
     * @brief sendInputMsgSignal
     * @param pdu
     */
    void sendInputMsgSignal(QString myName,QString friendName,QString inputMsg);

private slots:
    void on_sendMsg_pb_clicked();

private:
    Ui::PrivateChat *ui;

    QString myName;
    QString friendName;
};

#endif // PRIVATECHAT_H
