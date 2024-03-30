#include "book.h"
#include "widget.h"


#include <QDebug>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>

#include <QFileInfo>

#include <sharefile.h>

Book::Book(QWidget *parent)
    : QWidget{parent}
{
    m_pBookListW = new QListWidget;
    m_pReturnPB = new QPushButton("返回");
    m_pCreateDirDB = new QPushButton("创建文件夹");
    m_pRDelDirPB = new QPushButton("删除文件夹");
    m_pRenamePB = new QPushButton("重命名文件");
    m_pFlushFilePB = new QPushButton("刷新文件");
    m_pUploadPB = new QPushButton("上传文件");
    m_pDownloadPB = new QPushButton("下载文件");
    m_pDelFilePB = new QPushButton("删除文件");
    m_pShareFilePB = new QPushButton("共享文件");

    // 第一列按键
    QVBoxLayout *oneBtnV = new QVBoxLayout;
    oneBtnV->addWidget(m_pReturnPB);
    oneBtnV->addWidget(m_pCreateDirDB);
    oneBtnV->addWidget(m_pRDelDirPB);
    oneBtnV->addWidget(m_pRenamePB);
    oneBtnV->addWidget(m_pFlushFilePB);

    // 第二列
    QVBoxLayout *twoBtnV = new QVBoxLayout;
    twoBtnV->addWidget(m_pUploadPB);
    twoBtnV->addWidget(m_pDownloadPB);
    twoBtnV->addWidget(m_pDelFilePB);
    twoBtnV->addWidget(m_pShareFilePB);

    // 总水平布局
    QHBoxLayout *mainH = new QHBoxLayout;
    mainH->addWidget(m_pBookListW);
    mainH->addLayout(oneBtnV);
    mainH->addLayout(twoBtnV);

    setLayout(mainH);

    connect(m_pCreateDirDB,SIGNAL(clicked(bool))
            ,this,SLOT(onCreateDirBtnClicked()));

    connect(m_pFlushFilePB,SIGNAL(clicked(bool))
            ,this,SLOT(onFlushFilePBClicked()));

    connect(m_pRDelDirPB,SIGNAL(clicked(bool))
            ,this,SLOT(onDelDirPBClicked()));

    connect(m_pRenamePB,SIGNAL(clicked(bool))
            ,this,SLOT(onRenamePBClicked()));

    connect(m_pBookListW,SIGNAL(doubleClicked(QModelIndex))
            ,this,SLOT(onDoubleBookListItemEnterDir(QModelIndex)));

    connect(m_pReturnPB,SIGNAL(clicked(bool))
            ,this,SLOT(onReturnPBClicked()));

    connect(m_pUploadPB,SIGNAL(clicked(bool))
            ,this,SLOT(onUploadPBClicked()));

    connect(&m_timer,SIGNAL(timeout())
            ,this,SLOT(fileUploading()));
    m_timer.setSingleShot(true);


    connect(m_pDelFilePB,SIGNAL(clicked(bool))
            ,this,SLOT(onDelFilePBClicked()));


    connect(m_pDownloadPB,SIGNAL(clicked(bool))
            ,this,SLOT(onDownloadPBClicked()));

    connect(m_pShareFilePB,SIGNAL(clicked(bool))
            ,this,SLOT(onShareFilePBClicked()));

    connect(&ShareFile::getInstance(),SIGNAL(selectedFriendsSignal(QStringList))
            ,this,SLOT(handleShareFile(QStringList)));
}

Book::~Book()
{
    qDebug() << "book delete";
    m_pBookListW->clear();
    delete m_pBookListW;
    m_pBookListW = NULL;
    delete m_pReturnPB;
    m_pReturnPB = NULL;
    delete m_pCreateDirDB;
    m_pCreateDirDB = NULL;
    delete m_pRDelDirPB;
    m_pRDelDirPB = NULL;
    delete m_pRenamePB;
    m_pRenamePB = NULL;
    delete m_pFlushFilePB;
    m_pFlushFilePB = NULL;
    delete m_pUploadPB;
    m_pUploadPB = NULL;
    delete m_pDownloadPB;
    m_pDownloadPB = NULL;
    delete m_pDelFilePB;
    m_pDelFilePB = NULL;
    delete m_pShareFilePB;
    m_pShareFilePB = NULL;
}

void Book::addFileToBookList(int fileType, QString fileName)
{
    QListWidgetItem *item = new QListWidgetItem;
    if(fileType == FILE_TYPE_IS_DIR)
    {
        item->setIcon(QPixmap(":/images/dir.png"));
    }
    else if(fileType == FILE_TYPE_IS_FILE)
    {
        item->setIcon(QPixmap(":/images/reg.png"));
    }

    item->setText(fileName);
    m_pBookListW->addItem(item);
}

void Book::showFilesToBookList(const PDU *pdu)
{
    int fileCount = pdu->uiMsgLen/sizeof(FileInfo);
    FileInfo *pFileInfo = NULL;
    for(int i=0;i<fileCount;i++)
    {
        pFileInfo = (FileInfo*)(pdu->caMsg)+i;
       QListWidgetItem *item = new QListWidgetItem;
        if(pFileInfo->fileType == FILE_TYPE_IS_DIR)
        {
            item->setIcon(QPixmap(":/images/dir.png"));
        }
        else if(pFileInfo->fileType == FILE_TYPE_IS_FILE)
        {
            item->setIcon(QPixmap(":/images/reg.png"));
        }
        item->setText(pFileInfo->fileName);
        m_pBookListW->addItem(item);
    }
}

void Book::clearBookList()
{
    m_pBookListW->clear();
}

void Book::onCreateDirBtnClicked()
{
    // 获取用户名
    QString usrName = Widget::getInstance().getUsrName();
    // 获取当前目录路径
    QString currentPath = Widget::getInstance().getCurrentPath();
    // 通过弹窗获取要创建的目录名
    QString newDir = QInputDialog::getText(this,"创建目录","输入新建目录名");
    if(newDir.size()>=32)
    {
        QMessageBox::warning(this,"警告","创建目录名过长");
        return;
    }
    // 封装发送PDU
    PDU *pdu = mkPDU(currentPath.size()+1);
    // 封装pdu消息类型
    pdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_REQUEST;
    // 封装用户名
    strncpy(pdu->caData,usrName.toStdString().c_str(),usrName.size());
    strncpy(pdu->caData+32,newDir.toStdString().c_str(),newDir.size());
    // 封装新目录创建路径
    memcpy((char*)(pdu->caMsg),currentPath.toStdString().c_str(),currentPath.size());
    // 发送
    Widget::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Book::onFlushFilePBClicked()
{
    // 获取当前目录路径
    QString currentPath = Widget::getInstance().getCurrentPath();
    PDU *pdu = mkPDU(currentPath.size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_REQUEST;
    strncpy((char*)pdu->caMsg,currentPath.toStdString().c_str(),currentPath.size());
    Widget::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
    // qDebug() << " currentPath=" << currentPath;
    // qDebug() << "flush start";
}

void Book::onDelDirPBClicked()
{
    // 判空
    if(m_pBookListW->currentItem()==nullptr)
    {
        QMessageBox::warning(this,"删除文件夹","没有选择要删除的文件夹");
        return ;
    }

    // 获取当前目录路径
    QString curPath = Widget::getInstance().getCurrentPath();
    // 获取选择待删除的文件夹名
    QString delDirName = m_pBookListW->currentItem()->text();


    PDU *pdu = mkPDU(curPath.size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_DELETE_DIR_REQUEST;
    memcpy((char*)pdu->caMsg,curPath.toStdString().c_str(),curPath.size());
    strncpy(pdu->caData,delDirName.toStdString().c_str(),32);
    Widget::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Book::onRenamePBClicked()
{
    // 判空
    if(m_pBookListW->currentItem()==nullptr)
    {
        QMessageBox::warning(this,"文件重命名","没有选择要重命名的文件夹或文件");
        return ;
    }
    // 获取当前目录路径
    QString curPath = Widget::getInstance().getCurrentPath();
    // 获取旧的文件名
    QString oldName = m_pBookListW->currentItem()->text();
    // 获取新的名字
    QString newName = QInputDialog::getText(this,"文件重命名","新文件名（英文，小于32字符）");
    if(newName.size() > 32)
    {
        QMessageBox::warning(this,"文件重命名","新文件名大于32字符");
        return ;
    }

    PDU *pdu = mkPDU(curPath.size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_RENAME_FILE_REQUEST;
    strncpy(pdu->caData,oldName.toStdString().c_str(),oldName.size());
    strncpy(pdu->caData+32,newName.toStdString().c_str(),newName.size());
    memcpy((char*)pdu->caMsg,curPath.toStdString().c_str(),curPath.size());
    Widget::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Book::onDoubleBookListItemEnterDir(const QModelIndex index)
{
    QString fileName = index.data().toString();
    // 获取当前目录路径
    QString curPath = Widget::getInstance().getCurrentPath();

    PDU *pdu = mkPDU(curPath.size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_ENTER_DIR_REQUEST;
    memcpy((char*)pdu->caMsg,curPath.toStdString().c_str(),curPath.size());
    strncpy(pdu->caData,fileName.toStdString().c_str(),fileName.size());

    Widget::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Book::onReturnPBClicked()
{
    // 判断是否为rootDIr
    QFile configFile(":/client.config");
    QString rootDir;
    if(configFile.open(QIODevice::ReadOnly))
    {
        QByteArray info = configFile.readAll();
        configFile.close();
        QString infoStr = info.toStdString().c_str();
        infoStr.replace("\r\n"," ");
        QStringList infoList = infoStr.split(" ");
        rootDir = infoList.at(2);
    }
    rootDir.append("/").append(Widget::getInstance().getUsrName());
    QString curPath = Widget::getInstance().getCurrentPath();
    if(curPath==rootDir)
    {
        QMessageBox::warning(this,"返回上一级","这是根目录");
        return;
    }
    // 修改Widget::m_curPath

    int index = curPath.lastIndexOf("/");
    curPath.remove(index,curPath.size()-index);
    Widget::getInstance().setCurrentPath(curPath);

    // 刷新
    onFlushFilePBClicked();
}

void Book::onUploadPBClicked()
{
    // 清空m_uploadFilePath前面记录的路径
    m_uploadFilePath = nullptr;
    // 获取上传文件路径
    m_uploadFilePath = QFileDialog::getOpenFileName(this,".");
    if(m_uploadFilePath == nullptr)
    {
        QMessageBox::warning(this,"上传文件","未选择文件");
        return;
    }

    // 获取文件名、和文件大小
    QFileInfo fileInfo(m_uploadFilePath);
    QString fileName = fileInfo.fileName();
    qint64 fileSize = fileInfo.size();

    // 获取当前网盘路径
    QString curPath = Widget::getInstance().getCurrentPath();

    PDU *pdu = mkPDU(curPath.size()+1);
    // 封装文件名，和文件大小
    sprintf(pdu->caData,"%s %lld",fileName.toStdString().c_str(),fileSize);
    // 封装当前路径
    memcpy((char*)pdu->caMsg,curPath.toStdString().c_str(),curPath.size());

    pdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST;

    // 发送
    Widget::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu = NULL;

    // 设置定时器，计数结束后自动上传文件
    // 目的是给服务器处理的时间
    m_timer.start(2000);

}

void Book::fileUploading()
{
    // QMessageBox::information(this,"上传文件",m_uploadFilePath);

    // 打开需要上传的文件
    QFile file(m_uploadFilePath);
    if(file.open(QIODevice::ReadOnly))
    {
        char *buffer = new char[4096];
        qint64 ret = 0;
        while(true)
        {
            ret = file.read(buffer,4096);
            if(ret > 0 && ret <= 4096)
            {
                Widget::getInstance().getTcpSocket().write(buffer,4096);
                Widget::getInstance().getTcpSocket().flush();
            }
            else if(0==ret)
            {
                // 读取结束,退出
                break;
            }
            else
            {
                QMessageBox::warning(this,"上传失败","服务器不能结束接收状态，程序不能正常运行");
                break;
            }
        }
        file.close();
        delete []buffer;
        buffer = NULL;
    }
}

void Book::onDelFilePBClicked()
{
    // 获取当前目录路径
    QString curPath = Widget::getInstance().getCurrentPath();
    // 获取当前选中的文件名
    if( m_pBookListW->currentItem() == nullptr)
    {
        QMessageBox::warning(this,"删除文件","未选中文件");
        return;
    }
    QString fileName = m_pBookListW->currentItem()->text();


    PDU *pdu = mkPDU(curPath.size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FILE_REQUEST;
    strncpy(pdu->caData,fileName.toStdString().c_str(),fileName.size());
    memcpy((char*)pdu->caMsg,curPath.toStdString().c_str(),curPath.size());

    // 发送pdu
    Widget::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu = NULL;

}

void Book::onDownloadPBClicked()
{
    // 判断是否选择了文件
    if( m_pBookListW->currentItem() == nullptr)
    {
        QMessageBox::warning(this,"下载文件","未选中文件");
        return;
    }
    // 获取要下载的文件名
    QString fileName = m_pBookListW->currentItem()->text();
    emit downloadSignal(fileName);
}

void Book::onShareFilePBClicked()
{
    if(m_pBookListW->currentItem() == nullptr)
    {
        QMessageBox::warning(this,"分享文件","未选中文件");
        return;
    }
    shareFileName = m_pBookListW->currentItem()->text();

    QStringList friendList = OpeWidget::getInstance().getOnlineFriends();
    ShareFile::getInstance().flushButtonGroup(friendList);
    ShareFile::getInstance().show();


}

void Book::handleShareFile(QStringList friends)
{
    int num = friends.count();
    QString curPath = Widget::getInstance().getCurrentPath();
    curPath.append("/");
    curPath.append(shareFileName);
    QString usrName = Widget::getInstance().getUsrName();
    PDU *pdu = mkPDU(num*32+curPath.size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_SHARE_REQUEST;
    for(int i=0;i<num;i++)
    {
        memcpy((char*)(pdu->caMsg)+i*32,friends[i].toStdString().c_str(),32);
    }
    memcpy((char*)(pdu->caMsg)+num*32,curPath.toStdString().c_str(),curPath.size());
    sprintf(pdu->caData,"%s %d",usrName.toStdString().c_str(),num);

    Widget::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}
