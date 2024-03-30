#include "widget.h"
#include "ui_widget.h"
#include <QFile>
#include <QMessageBox>
#include <QDebug>
#include <QDir>
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    // 加载配置文件
    loadingConfig();

    QDir rootDir;
    if(!rootDir.exists(m_rootDir))
    {
        bool ret = rootDir.mkdir(m_rootDir);
        if(!ret)
        {
             QMessageBox::warning(this,"错误","系统文件创建错误");
        }
    }

    // 监听
    MyTcpServer::getInstance().listen(QHostAddress(m_strIP),m_usPort);
}

Widget::~Widget()
{
    delete ui;
}

QString Widget::getUsrRootDirPath()
{
    return m_rootDir;
}

Widget &Widget::getInstance()
{
    static Widget instance;
    return instance;
}

void Widget::loadingConfig()
{
    QFile configFile(":/server.config");
    if(configFile.open(QIODevice::ReadOnly))
    {
        QByteArray info = configFile.readAll();
        configFile.close();
        QString infoStr = info.toStdString().c_str();
        infoStr.replace("\r\n"," ");
        QStringList infoList = infoStr.split(" ");

        m_strIP = infoList.at(0);
        m_usPort = infoList.at(1).toUInt();
        m_rootDir = infoList.at(2);
        qDebug() << m_strIP << " " << m_usPort << " " << m_rootDir;

    }
    else{
        QMessageBox::critical(this,"config open","config open err");
    }
}
