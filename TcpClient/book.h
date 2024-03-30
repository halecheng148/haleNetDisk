#ifndef BOOK_H
#define BOOK_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include "protocol.h"
class Book : public QWidget
{
    Q_OBJECT
public:
    explicit Book(QWidget *parent = nullptr);
    ~Book();

    void addFileToBookList(int fileType,QString fileName);

    void showFilesToBookList(const PDU *pdu);

    void clearBookList();
signals:
    void downloadSignal(QString downloadFile);

public slots:

    /**
     * 请求创建新的目录
     * @brief onCreateDirBtnClicked
     */
    void onCreateDirBtnClicked();

    /**
     * 请求刷新显示当前目录下所有文件
     * @brief onFlushFilePBClicked
     */
    void onFlushFilePBClicked();

    /**
     * 删除文件夹的按钮绑定函数,用于发送删除文件夹请求
     * @brief onDelDirPBClicked
     */
    void onDelDirPBClicked();

    /**
     * 重命名文件请求，m_pRenamePB绑定
     * @brief onRenamePBClicked
     */
    void onRenamePBClicked();

    /**
     * 双击m_pBookListW中的文件夹，进入文件夹
     * @brief onDoubleBookListItemEnterDir
     */
    void onDoubleBookListItemEnterDir(const QModelIndex index);

    /**
     * 返回上一级目录
     * @brief onReturnPBClicked
     */
    void onReturnPBClicked();

    /**
     * 上传按钮的槽函数，
     * 获取要上传的文件路径，记录在m_uploadFilePath中
     * 提取文件名、获取文件大小
     * 封装PDU，发送
     * @brief onUploadPBClicked
     */
    void onUploadPBClicked();


    /**
     * 开始上传文件
     * @brief fileUploading
     */
    void fileUploading();


    /**
     * 删除文件槽函数
     * @brief onDelFilePBClicked
     */
    void onDelFilePBClicked();

    /**
     * 下载文件按钮槽函数
     * 获取要下载的文件名、文件所在目录
     * 选择保存文件的路径，保存再
     *
     * @brief onDownloadPBClicked
     */
    void onDownloadPBClicked();

    /**
     * 共享文件，检查是否选择了共享的文件
     * 获取当前在线的好友list
     * 将好友list传给sharefile窗口
     * 显示sharefile窗口
     * @brief onShareFilePBClicked
     */
    void onShareFilePBClicked();


    void handleShareFile(QStringList friends);

private:
    QListWidget *m_pBookListW;      // 文件列表
    QPushButton *m_pReturnPB;       // 返回按钮
    QPushButton *m_pCreateDirDB;    // 创建文件夹
    QPushButton *m_pRDelDirPB;      // 删除文件夹
    QPushButton *m_pRenamePB;       // 重命名文件
    QPushButton *m_pFlushFilePB;    // 刷新文件
    QPushButton *m_pUploadPB;       // 上次文件
    QPushButton *m_pDownloadPB;     // 下载文件
    QPushButton *m_pDelFilePB;      // 删除文件
    QPushButton *m_pShareFilePB;    // 共享文件

    QString     m_uploadFilePath;   // 上传文件路径
    QTimer      m_timer;            // 定时器

    QString     shareFileName;      // 分享文件的名字

};

#endif // BOOK_H
