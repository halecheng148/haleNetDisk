#include "widget.h"
#include "ui_widget.h"
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include <QHostAddress>
#include "privatechat.h"
// #include "protocol.h"
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    LoadingConfig();

    // 连接成功弹窗
    connect(&m_socket,SIGNAL(connected()),this,SLOT(showConnected()));

    // 接收服务器端的注册响应
    connect(&m_socket,SIGNAL(readyRead()),this,SLOT(recvMsg()));

    // 连接服务器,异步
    m_socket.connectToHost(QHostAddress(m_strIP),m_usPort);


    connect(OpeWidget::getInstance().getBookUI(),SIGNAL(downloadSignal(QString))
            ,this,SLOT(handleDownload(QString)));
}

Widget::~Widget()
{
    qDebug() << " widget quit";


    delete ui;
}

Widget &Widget::getInstance()
{
    static Widget instance;
    return instance;
}

QTcpSocket& Widget::getTcpSocket()
{
    return m_socket;
}

void Widget::LoadingConfig()
{
    QFile configFile(":/client.config");
    if(configFile.open(QIODevice::ReadOnly))
    {
        QByteArray info = configFile.readAll();
        configFile.close();
        QString infoStr = info.toStdString().c_str();
        infoStr.replace("\r\n"," ");
        QStringList infoList = infoStr.split(" ");

        m_strIP = infoList.at(0);
        m_usPort = infoList.at(1).toUInt();
        m_curPath = infoList.at(2);
        qDebug() << m_strIP << " " << m_usPort <<" " <<m_curPath;

    }
    else{
        QMessageBox::critical(this,"config open","config open err");
    }
}

QString Widget::getUsrName()
{
    return this->m_usrName;
}

QString Widget::getCurrentPath()
{
    return m_curPath;
}

void Widget::setCurrentPath(QString curPath)
{
    this->m_curPath = curPath;
}

void Widget::showConnected()
{
    QMessageBox::information(this,"连接服务器","连接服务器成功");
}

void Widget::recvMsg()
{
    if(!m_isDownload)
    {
        uint uiPDULen = 0;
        m_socket.read((char*)&uiPDULen,sizeof(uint));

        uint uiMsgLen = uiPDULen-sizeof(PDU);

        PDU *pdu = mkPDU(uiMsgLen);

        m_socket.read((char*)pdu+sizeof(uint),uiPDULen-sizeof(uint));
        switch (pdu->uiMsgType) {
        case ENUM_MSG_TYPE_REGIST_RESPOND:
        {
            if(0 == strcmp(pdu->caData,REGIST_OK))
            {
                QMessageBox::information(this,"注册信息",REGIST_OK);
            }
            else if(0 == strcmp(pdu->caData,REGIST_FAILED))
            {
                QMessageBox::warning(this,"警告",REGIST_FAILED);
            }
            break;
        }
        case ENUM_MSG_TYPE_LOGIN_RESPOND:
        {
            if(0 == strcmp(pdu->caData,LOGIN_OK))
            {
                QMessageBox::information(this,"登录信息",LOGIN_OK);
                OpeWidget::getInstance().show();
                this->hide();
                m_usrName = ui->name_le->text();
                m_curPath =m_curPath + "/"+m_usrName;


            }
            else
            {
                QMessageBox::warning(this,"登录信息",LOGIN_FAILED);
                m_usrName = "";
            }
            break;
        }
        case ENUM_MSG_TYPE_ALL_ONLINE_RESPOND:
        {
            QStringList nameList;
            int num = pdu->uiMsgLen/32;
            for(int i = 0;i<num;i++)
            {
                char str[32];
                memcpy(str,(char*)(pdu->caMsg)+i*32,32);
                nameList.append(str);
            }
            OpeWidget::getInstance().setFriendOnlineShowALl(nameList);
            break;
        }
        case ENUM_MSG_TYPE_SEARCH_RESPOND:
        {
            QString name = OpeWidget::getInstance().getFriendUI()->m_currentSearchName;
            if(0 == strcmp(pdu->caData,SEARCH_USR_NO))
            {
                name=name+":"+SEARCH_USR_NO;
                QMessageBox::information(this,"查询结果",name);
            }
            else if(0 == strcmp(pdu->caData,SEARCH_USR_OFFLINE))
            {
                name=name+":"+SEARCH_USR_OFFLINE;
                QMessageBox::information(this,"查询结果",name);
            }
            else if(0 == strcmp(pdu->caData,SEARCH_USR_ONLINE))
            {
                name=name+":"+SEARCH_USR_ONLINE;
                QMessageBox::information(this,"查询结果",name);
            }
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:
        {
            // 添加好友的申请
            char friendName[32] = {'\0'};
            strncpy(friendName,pdu->caData+32,32);
            QMessageBox::StandardButton ret = QMessageBox::information(this,"好友申请通知",QString("%1想添加你为好友").arg(friendName)
                                                                       ,QMessageBox::Yes,QMessageBox::No);

            PDU* respdu = mkPDU(0);
            memcpy(respdu->caData,pdu->caData,64);
            if(ret == QMessageBox::Yes)
            {
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_AGGREE;
            }
            else
            {
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REFUSE;
            }
            m_socket.write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_RESPOND:
        {
            // 添加失败
            QMessageBox::information(this,"添加好友",pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_AGGREE:
        {
            // 添加成功，通知
            char friendName[32] ={'\0'};
            strncpy(friendName,pdu->caData,32);
            QString info = QString("与 %1 成为好友").arg(friendName);
            QMessageBox::information(this,"成功添加好友",info);
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_REFUSE:
        {
            // 添加失败通知
            char friendName[32] ={'\0'};
            strncpy(friendName,pdu->caData,32);
            QString info = QString(" %1 拒绝了你的好友申请").arg(friendName);
            QMessageBox::information(this,"添加好友失败",info);
            break;
        }
        case ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND:
        {
            int len = uiMsgLen/32;
            QStringList nameList;
            char name[32] = {'\0'};
            for(int i=0;i<len;i++)
            {
                memcpy(name,(char*)(pdu->caMsg)+i*32,32);
                nameList.append(name);
            }
            OpeWidget::getInstance().getFriendUI()->showOnlineFriend(nameList);
            break;
        }
        case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST:
        {
            // 被好友删除时
            char friendName[32] = {'\0'};
            strncpy(friendName,pdu->caData+32,32);
            QMessageBox::information(this,"通知",QString("被 %1 删除好友关系").arg(friendName));
            break;
        }
        case ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND:
        {
            QMessageBox::information(this,"通知",pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:
        {
            // 接收另一个客户端发送的私聊消息
            char friendName[32] = {'\0'};
            strncpy(friendName,pdu->caData,32);
            if(PrivateChat::getInstance().isHidden())
            {
                PrivateChat::getInstance().show();
                PrivateChat::getInstance().setFriendName(friendName);
                PrivateChat::getInstance().setMyName(getUsrName());
            }
            QString sendMsg = friendName;
            sendMsg = sendMsg + " said: "+(char*)pdu->caMsg;
            PrivateChat::getInstance().appendShowMsgTextEdit(sendMsg);
            break;
        }
        case ENUM_MSG_TYPE_GROUP_CHAT_RESPOND:
        {
            char friendName[32] = {'\0'};
            strncpy(friendName,pdu->caData,32);
            QString msg = friendName;
            msg = msg +" said: " + (char*)pdu->caMsg;
            OpeWidget::getInstance().getFriendUI()->showRecvGroupChatMsg(msg);
            break;
        }
        case ENUM_MSG_TYPE_CREATE_DIR_RESPOND:
        {
            QMessageBox::information(this,"创建目录响应",pdu->caData);
            // 发送刷新请求
            OpeWidget::getInstance().getBookUI()->onFlushFilePBClicked();
            break;
        }
        case ENUM_MSG_TYPE_FLUSH_FILE_RESPOND:
        {
            // qDebug() << "show flush end";
            // 读取pdu 将当前目录下的文件显示在book::m_pBookListW中
            OpeWidget::getInstance().getBookUI()->clearBookList();
            OpeWidget::getInstance().getBookUI()->showFilesToBookList(pdu);
            // int fileCount = pdu->uiMsgLen/sizeof(FileInfo);
            // FileInfo *pFileInfo = NULL;
            // for(int i=0;i<fileCount;i++)
            // {
            //     pFileInfo = (FileInfo*)(pdu->caMsg)+i;
            //     OpeWidget::getInstance().getBookUI()->addFileToBookList(pFileInfo->fileType,pFileInfo->fileName);
            // }
            break;
        }
        case ENUM_MSG_TYPE_DELETE_DIR_RESPOND:
        {
            // 删除回应
            // 解析pdu ,弹窗显示信息
            QMessageBox::information(this,"删除文件夹",pdu->caData);
            // 发送刷新请求
            OpeWidget::getInstance().getBookUI()->onFlushFilePBClicked();
            break;
        }
        case ENUM_MSG_TYPE_RENAME_FILE_RESPOND:
        {
            // 删除回应
            // 解析pdu ,弹窗显示信息
            QMessageBox::information(this,"删除文件夹",pdu->caData);
            // 发送刷新请求
            OpeWidget::getInstance().getBookUI()->onFlushFilePBClicked();
            break;
        }
        case ENUM_MSG_TYPE_ENTER_DIR_RESPOND:
        {
            // 这里使用了uiMsgLen记录一个FILE_TYPE枚举类型的值
            int type = pdu->uiMsgLen;
            if(type==FILE_TYPE_IS_DIR)
            {
                // 获取要进入的目录名
                char dirName[32] = {'\0'};
                strncpy(dirName,pdu->caData,32);
                // 获取当前路径
                QString curPath = getCurrentPath();
                // 修改当前目录路径
                curPath = curPath + "/" +dirName;
                setCurrentPath(curPath);
                // 刷新目录
                OpeWidget::getInstance().getBookUI()->onFlushFilePBClicked();
            }
            else
            {
                QMessageBox::warning(this,"进入目录","这不是一个目录");
            }
            break;
        }
        case ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND:
        {
            // 上传结果，显示
            QMessageBox::information(this,"上传文件",pdu->caData);
            OpeWidget::getInstance().getBookUI()->onFlushFilePBClicked();
            break;
        }
        case ENUM_MSG_TYPE_DELETE_FILE_ISDIR:
        {
            // 是目录，不能删除
            QMessageBox::warning(this,"删除文件","目录删除请选择<删除文件夹>按钮");
            break;
        }
        case ENUM_MSG_TYPE_DELETE_FILE_RESPOND:
        {
            // 读取删除的文件名
            char fileName[32] = {'\0'};
            strncpy(fileName,pdu->caData,32);
            QString msg = QString("%1 : 文件已删除").arg(fileName);
            QMessageBox::information(this,"删除文件",msg);
            OpeWidget::getInstance().getBookUI()->onFlushFilePBClicked();
            break;
        }
        case ENUM_MSG_TYPE_DOWNLOAD_FILE_START:
        {
            // 获取服务器响应，判断是否进入下载文件状态
            int fileType = pdu->uiMsgLen;
            if(fileType == FILE_TYPE_IS_FILE)
            {
                // 打开或创建一个接收的文件
                m_saveFile.setFileName(m_saveFilePath);
                if(m_saveFile.open(QIODevice::WriteOnly))
                {
                    // 读取文件大小
                    sscanf(pdu->caData,"%lld",&m_total);
                    m_recved = 0;

                    // 设置下载标识
                    m_isDownload = true;
                }
                else
                {
                    QMessageBox::warning(this,"下载文件","系统繁忙");
                }
            }
            else
            {
                QMessageBox::warning(this,"下载文件","选择的是一个目录");
            }
            break
        }
        case ENUM_MSG_TYPE_SHARE_NOTE:
        {
            char sendName[32] = {'\0'};
            memcpy(sendName,pdu->caData,32);
            char *sharePath = new char[pdu->uiMsgLen];
            memcpy(sharePath,(char*)(pdu->caMsg),pdu->uiMsgLen);
            QString path = sharePath;
            int index = path.lastIndexOf("/");
            QString fileName = path.last(path.size()-index-1);
            QString msg = QString(sendName)+"分享了"+fileName;
            int ret = QMessageBox::question(this,"分享文件",msg);
            if(ret == QMessageBox::Yes)
            {
                PDU *respdu = mkPDU(pdu->uiMsgLen);
                memcpy(respdu->caMsg,pdu->caMsg,pdu->uiMsgLen);
                respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_NOTE_RESPOND;
                strncpy(respdu->caData,getUsrName().toStdString().c_str(),32);
                m_socket.write((char*)respdu,respdu->uiPDULen);
                free(respdu);
                respdu = NULL;
            }
            break;
        }
        case ENUM_MSG_TYPE_SHARE_RESPOND:
        {
            QMessageBox::information(this,"分享文件","分享成功获取");
            break;
        }
        default:
            break;
        }

        free(pdu);
        pdu = NULL;
    }
    else
    {
        QByteArray buff = m_socket.readAll();
        m_saveFile.write(buff);
        m_recved += buff.size();
        if(m_total <= m_recved)
        {
            if(m_total == m_recved)
            {
                m_saveFile.close();
                m_isDownload = false;
                QMessageBox::warning(this,"下载文件","下载完成");
            }
            else if(m_total < m_recved)
            {
                m_saveFile.close();
                m_isDownload = false;
                QMessageBox::warning(this,"下载文件","下载失败");
            }
        }
    }
}

#if 0
void Widget::on_pushButton_clicked()
{
    QString strMsg = ui->lineEdit->text();
    if(!strMsg.isEmpty())
    {
        PDU *pdu = mkPDU(strMsg.size()+1);
        // 设置消息类型
        pdu->uiMsgType = 1;
        // 封装消息
        strcpy((char*)pdu->caMsg,strMsg.toStdString().c_str());
        qDebug() << (char*)pdu->caMsg;
        // 发送消息
        m_socket.write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
    else
    {
        QMessageBox::information(this,"发送消息","发送失败");
    }
}
#endif


void Widget::on_regist_pb_clicked()
{
    QString strName = ui->name_le->text();
    QString strPwd = ui->pwd_le->text();

    if(!strName.isEmpty() && !strPwd.isEmpty())
    {
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_REGIST_REQUEST;
        strncpy(pdu->caData,strName.toStdString().c_str(),32);
        strncpy(pdu->caData+32,strPwd.toStdString().c_str(),32);
        m_socket.write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }else{
        QMessageBox::warning(this,"警告","密码或账户不能为空");
    }
}


void Widget::on_login_pb_clicked()
{
    QString strName = ui->name_le->text();
    QString strPwd = ui->pwd_le->text();

    if(!strName.isEmpty() && !strPwd.isEmpty())
    {
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_REQUEST;
        strncpy(pdu->caData,strName.toStdString().c_str(),32);
        strncpy(pdu->caData+32,strPwd.toStdString().c_str(),32);
        m_socket.write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
    else
    {
        QMessageBox::warning(this,"警告","密码或账户不能为空");
    }
}

void Widget::handleDownload(QString downloadFile)
{
    // 获取保存路径
    m_saveFilePath = QFileDialog::getSaveFileName(this,"Save File",downloadFile);
    // 获取当前路径
    QString curPath = getCurrentPath();
    PDU *pdu = mkPDU(curPath.size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST;
    // 将当前路径和文件名封装PDU
    strncpy(pdu->caData,downloadFile.toStdString().c_str(),downloadFile.size());
    memcpy((char*)pdu->caMsg,curPath.toStdString().c_str(),curPath.size());

    m_socket.write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

