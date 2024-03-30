#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpSocket>
#include <QFile>
#include "opewidget.h"
QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    static Widget& getInstance();

    QTcpSocket& getTcpSocket();

    /**
     * @brief LoadingConfig 加载配置文件
     * 初始化 m_strIp 和 m_usPort
     */
    void LoadingConfig();

    /**
     * @brief getUsrName
     * @return  返回当前登录的用户名
     */
    QString getUsrName();



    /**
     * 获取当前目录
     * @brief getCurrentPath
     * @return
     */
    QString getCurrentPath();

    /**
     * 设置当前目录
     * @brief setCurrentPath
     * @param curPath
     */
    void setCurrentPath(QString curPath);

private slots:
    /**
     * @brief showConnected 弹窗显示是否成功连接
     */
    void showConnected();

    /**
     * @brief recvMsg 接收数据
     */
    void recvMsg();


    // void on_pushButton_clicked();

    void on_regist_pb_clicked();

    void on_login_pb_clicked();


    void handleDownload(QString downloadFile);

private:
    Ui::Widget *ui;
    QString m_strIP;                // IP地址
    quint16 m_usPort;               // 端口号
    QTcpSocket m_socket;            // tcp套接字，连接服务器

    QString m_usrName;              // 登录用户名

    QString m_curPath;              // 当前目录路径

    QString m_saveFilePath;         // 保存下载文件的地址
    qint64 m_total;                 // 下载文件大小
    qint64 m_recved;                // 已经下载的大小

    QFile m_saveFile;               // 为下载文件打开的文件
    bool m_isDownload=false;              // 下载文件的标识

};
#endif // WIDGET_H
