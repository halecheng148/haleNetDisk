#include "opewidget.h"
#include "widget.h"
OpeWidget::OpeWidget(QWidget *parent)
    : QWidget{parent}
{
    m_pListW = new QListWidget;
    m_pListW->addItem("好友");
    m_pListW->addItem("图书");

    m_friend = new Friend;
    m_book = new Book;

    m_stackW = new QStackedWidget;
    m_stackW->addWidget(m_friend);
    m_stackW->addWidget(m_book);

    QHBoxLayout *pMain = new QHBoxLayout;
    pMain->addWidget(m_pListW);
    pMain->addWidget(m_stackW);

    setLayout(pMain);

    connect(m_pListW,SIGNAL(currentRowChanged(int))
            ,m_stackW,SLOT(setCurrentIndex(int)));

}

OpeWidget::~OpeWidget()
{
    qDebug() << "opeWidget delete";
    m_pListW->clear();
    delete m_pListW;
    m_pListW = NULL;

    delete m_friend;
    m_friend = NULL;
    delete m_book;
    m_book = NULL;

    delete m_stackW;
}

void OpeWidget::setFriendOnlineShowALl(QStringList nameList)
{
    m_friend->showAllOnline(nameList);
}

Friend *OpeWidget::getFriendUI()
{
    return m_friend;
}

Book *OpeWidget::getBookUI()
{
    return m_book;
}

OpeWidget &OpeWidget::getInstance()
{
    static OpeWidget instance;
    return instance;
}

QStringList OpeWidget::getOnlineFriends()
{
    return m_friend->getOnlineFriends();
}
