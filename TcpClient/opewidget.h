#ifndef OPEWIDGET_H
#define OPEWIDGET_H

#include <QWidget>
#include <QListWidget>
#include "friend.h"
#include "book.h"
#include <QStackedWidget>
class OpeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit OpeWidget(QWidget *parent = nullptr);
    ~OpeWidget();

    void setFriendOnlineShowALl(QStringList nameList);

    Friend* getFriendUI();

    Book* getBookUI();

    static OpeWidget& getInstance();

    QStringList getOnlineFriends();

signals:

public slots:

private:
    QListWidget      *m_pListW;     // 操作stackedWidget

    QStackedWidget   *m_stackW;     // 显示content

    Friend           *m_friend;     // 好友窗口

    Book             *m_book;       // 书籍窗口
};

#endif // OPEWIDGET_H
