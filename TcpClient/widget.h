#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpSocket>

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

    /**
     * @brief LoadingConfig 加载配置文件
     * 初始化 m_strIp 和 m_usPort
     */
    void LoadingConfig();

private slots:
    /**
     * @brief showConnected 弹窗显示是否成功连接
     */
    void showConnected();
    void on_pushButton_clicked();

private:
    Ui::Widget *ui;
    QString m_strIP;                // IP地址
    quint16 m_usPort;               // 端口号
    QTcpSocket m_socket;            // tcp套接字，连接服务器
};
#endif // WIDGET_H
