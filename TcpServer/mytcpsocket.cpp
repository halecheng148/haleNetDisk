#include "mytcpsocket.h"
#include <QDebug>
#include <QDir>
#include <QFileInfoList>
#include <widget.h>
MyTcpSocket::MyTcpSocket(QObject *parent)
    : QTcpSocket{parent}
{
    connect(this,SIGNAL(readyRead())
            ,this,SLOT(recvMsg()));

    connect(this,SIGNAL(disconnected())
            ,this,SLOT(clientOffline()));

    connect(&m_timer,SIGNAL(timeout())
            ,this,SLOT(downloadingFile()));
    m_timer.setSingleShot(true);
}

QString MyTcpSocket::getName()
{
    return strName;
}

void MyTcpSocket::recvMsg()
{
    if(!m_isUpload)
    {
        uint uiPDULen = 0;
        this->read((char*)&uiPDULen,sizeof(uint));

        uint uiMsgLen = uiPDULen-sizeof(PDU);

        PDU *pdu = mkPDU(uiMsgLen);

        this->read((char*)pdu+sizeof(uint),uiPDULen-sizeof(uint));
        switch (pdu->uiMsgType) {
        case ENUM_MSG_TYPE_REGIST_REQUEST:
        {
            char caName[32] = {'\0'};
            char caPwd[32] = {'\0'};
            strncpy(caName,pdu->caData,32);
            strncpy(caPwd,pdu->caData+32,32);
            bool ret = OpenDB::getInstance().handleRegist(caName,caPwd);
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_REGIST_RESPOND;
            if(ret)
            {
                strcpy(respdu->caData,REGIST_OK);

                // 注册成功，则创建用户目录
                QDir dirCreater;
                QString newDirPath = Widget::getInstance().getUsrRootDirPath() + "/" + caName;
                if(!dirCreater.exists(newDirPath))
                {
                    dirCreater.mkdir(newDirPath);
                }
            }else{
                strcpy(respdu->caData,REGIST_FAILED);
            }

            this->write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_LOGIN_REQUEST:
        {
            char caName[32] = {'\0'};
            char caPwd[32] = {'\0'};
            strncpy(caName,pdu->caData,32);
            strncpy(caPwd,pdu->caData+32,32);
            bool ret = OpenDB::getInstance().handleLogin(caName,caPwd);
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_RESPOND;
            if(ret)
            {
                strcpy(respdu->caData,LOGIN_OK);
                strName = caName;

            }
            else {
                strcpy(respdu->caData,LOGIN_FAILED);
            }
            this->write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_ALL_ONLINE_REQUEST:
        {
            QStringList nameList = OpenDB::getInstance().handleAllOnline();
            uint uiMsgLen = nameList.size()*32;
            PDU *respdu = mkPDU(uiMsgLen);
            respdu->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_RESPOND;
            for(int i = 0;i<nameList.length();i++)
            {
                memcpy((char*)(respdu->caMsg)+i*32
                       ,nameList.at(i).toStdString().c_str()
                       ,nameList.at(i).size());
            }
            this->write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_SEARCH_REQUEST:
        {


            char name[32] = {'\0'};
            memcpy(name,pdu->caData,32);
            int ret = OpenDB::getInstance().handleSearchUsr(name);


            qDebug() << pdu->caData;
            PDU* respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_SEARCH_RESPOND;
            if(ret==-1)
            {
                strcpy(respdu->caData,SEARCH_USR_NO);
            }
            else if(ret == 0)
            {
                strcpy(respdu->caData,SEARCH_USR_OFFLINE);
            }
            else if(ret == 1)
            {
                strcpy(respdu->caData,SEARCH_USR_ONLINE);
            }
            this->write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;

            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:
        {
            char friendName[32] = {'\0'};
            char myName[32] = {'\0'};
            memcpy(friendName,pdu->caData,32);
            memcpy(myName,pdu->caData+32,32);
            // 数据库操作
            int ret = OpenDB::getInstance().handleAddFriend(friendName,myName);
            PDU* respdu = mkPDU(0);
            switch (ret) {
            case -1:
            {
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
                strcpy(respdu->caData,UNKNOW_ERROR);
                this->write((char*)respdu,respdu->uiPDULen);
                free(respdu);
                respdu = NULL;
                break;
            }
            case 0:
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
                strcpy(respdu->caData,ADD_FRIEND_OFFLINE);
                this->write((char*)respdu,respdu->uiPDULen);
                free(respdu);
                respdu = NULL;
                break;
            case 1:
                // 转发请求，获取friend的同意
                emit addFriendRelaySignal(friendName,pdu);
                break;
            case 2:
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
                strcpy(respdu->caData,EXISTED_FRIEND);
                this->write((char*)respdu,respdu->uiPDULen);
                free(respdu);
                respdu = NULL;
                break;
            case 3:
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
                strcpy(respdu->caData,ADD_FRIEND_NOEXIST);
                this->write((char*)respdu,respdu->uiPDULen);
                free(respdu);
                respdu = NULL;
                break;
            default:
                break;
            }
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_AGGREE:
        {
            // 我同意你的好友申请
            PDU *respdu = mkPDU(0);
            // caData（my,you）
            char youName[32] = {'\0'};
            char myName[32] = {'\0'};
            memcpy(youName,pdu->caData+32,32);
            memcpy(myName,pdu->caData,32);
            memcpy(respdu->caData,pdu->caData,64);
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_AGGREE;
            emit addFriendRelaySignal(youName,respdu);

            memcpy(respdu->caData,youName,32);
            memcpy(respdu->caData+32,myName,32);
            emit addFriendRelaySignal(myName,respdu);
            // 数据库处理
            OpenDB::getInstance().handleInsertFriend(myName,youName);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_REFUSE:
        {
            // 我拒绝你的好友申请
            PDU *respdu = mkPDU(0);
            // caData（my,you）
            char youName[32] = {'\0'};
            memcpy(youName,pdu->caData+32,32);
            memcpy(respdu->caData,pdu->caData,64);
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REFUSE;
            emit addFriendRelaySignal(youName,respdu);


            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST:
        {
            char name[32] = {'\0'};
            strncpy(name,pdu->caData,32);   // 查询好友列表的用户名
            // 数据库操作
            QStringList nameList = OpenDB::getInstance().handleFlushFriend(name);
            uint uiMsgLen = nameList.size()*32;
            PDU *respdu = mkPDU(uiMsgLen);
            respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND;
            for(int i=0;i<nameList.length();i++)
            {
                memcpy((char*)(respdu->caMsg)+i*32
                       ,nameList.at(i).toStdString().c_str()
                       ,nameList.at(i).size());
            }
            this->write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST:
        {
            char myName[32] = {'\0'};
            char friendName[32] = {'\0'};
            strncpy(friendName,pdu->caData,32);
            strncpy(myName,pdu->caData+32,32);
            OpenDB::getInstance().handleDelFriend(myName,friendName);

            PDU* respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND;
            strcpy(respdu->caData,DEL_FRIEND_OK);
            this->write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;

            emit delFriendRelaySignal(friendName,pdu);
            break;
        }
        case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:
        {
            char friendName[32] = {'\0'};
            strncpy(friendName,pdu->caData+32,32);
            emit privateChatRelaySignal(friendName,pdu);
            break;
        }
        case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST:
        {
            // 读取pdu ,获取消息发送者的用户名

            char myName[32] = {'\0'};
            strncpy(myName,pdu->caData,32);
            // 搜索所有在线好友，获取在线好友列表
            QStringList friendList = OpenDB::getInstance().handleFlushFriend(myName);
            //循环转发群聊的pdu
            pdu->uiMsgType = ENUM_MSG_TYPE_GROUP_CHAT_RESPOND;
            for(const QString &friendName : friendList)
            {
                emit groupChatRelaySignal(friendName,pdu);
            }
            break;
        }
        case ENUM_MSG_TYPE_CREATE_DIR_REQUEST:
        {
            char *currPath = new char[pdu->uiMsgLen];
            memcpy(currPath,(char*)pdu->caMsg,pdu->uiMsgLen);

            char newDir[32] = {'\0'};
            strncpy(newDir,pdu->caData+32,32);

            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
            QDir curDir(currPath);
            if(curDir.exists())
            {
                // 路径存在
                // 查看目录下与新目录是否重名
                QString checkPath = QString("%1/%2").arg(currPath).arg(newDir);
                QDir checkDir(checkPath);
                if(!checkDir.exists())
                {
                    // 没有重名目录
                    // 创建目录
                    curDir.mkdir(newDir);
                    // 封装信息
                    strcpy(respdu->caData,CREATE_DIR_SUCCESS);
                }
                else {
                    // 目录创建重名
                    strcpy(respdu->caData,File_NAME_EXIST);
                }
            }
            else
            {
                strcpy(respdu->caData,DIR_NO_EXSIT);
            }
            this->write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;


            delete []currPath;
            currPath = NULL;
            break;
        }
        case ENUM_MSG_TYPE_FLUSH_FILE_REQUEST:
        {
            // qDebug() << "flush search start";
            char *curPath = new char[pdu->uiMsgLen];
            strncpy(curPath,(char*)pdu->caMsg,pdu->uiMsgLen);
            QDir dir(curPath);
            QFileInfoList fileInfoList = dir.entryInfoList();
            int fileCount = fileInfoList.count()-2;
            PDU* respdu = mkPDU(sizeof(FileInfo)*fileCount);
            respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_RESPOND;
            FileInfo *pFileInfo = NULL;
            QString strFileName;
            int index = 0;
            QList<QFileInfo>::const_iterator citer = fileInfoList.constBegin();
            for(;citer!=fileInfoList.constEnd();citer++)
            {
                strFileName = citer->fileName();
                if(strFileName == "." || strFileName == "..")
                {
                    continue;
                }

                pFileInfo = (FileInfo*)(respdu->caMsg)+index;
                memcpy(pFileInfo->fileName
                       ,strFileName.toStdString().c_str()
                       ,strFileName.size());
                if(citer->isDir())
                {
                    pFileInfo->fileType = FILE_TYPE_IS_DIR;
                }
                else
                {
                    pFileInfo->fileType = FILE_TYPE_IS_FILE;
                }
                index++;
            }
            this->write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            // qDebug() << "flush search over";


            delete []curPath;
            curPath = NULL;
            break;
        }
        case ENUM_MSG_TYPE_DELETE_DIR_REQUEST:
        {
            // 解析获取curPath、delDirName
            char *curPath = new char[pdu->uiMsgLen];
            char delDirName[32] = {'\0'};
            strncpy(delDirName,pdu->caData,32);
            memcpy(curPath,pdu->caMsg,pdu->uiMsgLen);
            // qDebug() << "curPath:" << curPath << " delDir:" << delDirName;

            // 删除文件夹
            QString path = QString(curPath)+QString("/")+QString(delDirName);
            QFileInfo fileInfo(path);
            bool ret = false;
            if(fileInfo.isDir())
            {
                QDir dir(path);
                ret = dir.removeRecursively(); // 递归删除，子文件夹和子文件
            }
            else
            {
                ret = false;
            }

            // 封装PDU
            PDU * respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_DIR_RESPOND;
            if(ret)
            {
                strncpy(respdu->caData,DELETE_DIR_OK,strlen(DELETE_DIR_OK));

            }
            else
            {
                strncpy(respdu->caData,DELETE_DIR_FAILURED,strlen(DELETE_DIR_FAILURED));
            }

            // 发送
            this->write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;


            delete []curPath;
            curPath = NULL;
            break;
        }
        case ENUM_MSG_TYPE_RENAME_FILE_REQUEST:
        {
            char *curPath = new char[pdu->uiMsgLen];
            memcpy(curPath,pdu->caMsg,pdu->uiMsgLen);
            char oldName[32] = {'\0'};
            char newName[32] = {'\0'};
            strncpy(oldName,pdu->caData,32);
            strncpy(newName,pdu->caData+32,32);
            QString oldPath = QString("%1/%2").arg(curPath).arg(oldName);
            QString newPath = QString("%1/%2").arg(curPath).arg(newName);

            QDir dir;
            bool ret = dir.rename(oldPath,newPath);

            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_RENAME_FILE_RESPOND;
            if(ret)
            {
                strncpy(respdu->caData,RENAME_FILE_OK,strlen(RENAME_FILE_OK));
            }
            else
            {
                strncpy(respdu->caData,RENAME_FILE_FAILURED,strlen(RENAME_FILE_FAILURED));
            }
            this->write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;


            delete []curPath;
            curPath = NULL;
            break;
        }
        case ENUM_MSG_TYPE_ENTER_DIR_REQUEST:
        {
            char dirName[32] = {'\0'};
            char *curPath = new char[pdu->uiMsgLen];
            strncpy(dirName,pdu->caData,32);
            memcpy(curPath,(char*)pdu->caMsg,pdu->uiMsgLen);

            QString path =QString("%1/%2").arg(curPath).arg(dirName);

            PDU *respdu;
            QFileInfo fileInfo(path);
            if(fileInfo.isDir())
            {
                // 是一个文件夹，可以进入
                respdu= mkPDU(FILE_TYPE_IS_DIR);
                strncpy(respdu->caData,dirName,32);
                respdu->uiMsgType = ENUM_MSG_TYPE_ENTER_DIR_RESPOND;
            }
            else
            {
                // 不是一个文件夹
                respdu= mkPDU(FILE_TYPE_IS_FILE);
                respdu->uiMsgType = ENUM_MSG_TYPE_ENTER_DIR_RESPOND;
            }
            this->write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;

            delete []curPath;
            curPath = NULL;
            break;
        }
        case ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST:
        {
            // 读取文件名、文件大小
            char fileName[32] = {'\0'};
            qint64 fileSize = 0;
            sscanf(pdu->caData,"%s %lld",fileName,&fileSize);
            // qDebug() << fileName << ":" << fileSize;

            // 读取保存路径，就是上传到路径
            char *savePath = new char[pdu->uiMsgLen];
            memcpy(savePath,(char*)pdu->caMsg,pdu->uiMsgLen);

            // 拼接长传文件路径
            QString path = QString("%1/%2").arg(savePath).arg(fileName);

            // 创建成员属性，保存文件大小，当前是否为上传状态，以及上传文件本身
            m_file.setFileName(path);
            // 没有文件，将自动创建文件
            if(m_file.open(QIODevice::WriteOnly))
            {
                // 进入上传文件的接收状态
                m_isUpload = true;
                m_Total = fileSize;
                m_Recved = 0;
            }

            delete []savePath;
            savePath = NULL;
            break;
        }
        case ENUM_MSG_TYPE_DELETE_FILE_REQUEST:
        {
            // 读取要删除的文件名、文件所在路径
            char *curPath = new char[pdu->uiMsgLen];
            memcpy(curPath,(char*)pdu->caMsg,pdu->uiMsgLen);
            char fileName[32] = {'\0'};
            strncpy(fileName,pdu->caData,32);

            // 拼接
            QString path = QString("%1/%2").arg(curPath).arg(fileName);

            // 判断、删除、封装pdu
            PDU* respdu = mkPDU(0);
            QFileInfo info(path);
            if(info.isFile())
            {
                // 是文件删除
                QDir dir;
                dir.remove(path);
                respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FILE_RESPOND;
                strncpy(respdu->caData,pdu->caData,32);

            }
            else
            {
                respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FILE_ISDIR;
            }

            // 发送
            this->write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;

            delete []curPath;
            curPath = NULL;
            break;
        }
        case ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST:
        {
            // 获取文件名、文件所在路径
            char *curPath = new char[pdu->uiMsgLen];
            memcpy(curPath,(char*)pdu->caMsg,pdu->uiMsgLen);
            char fileName[32] = {'\0'};
            strncpy(fileName,pdu->caData,32);

            // 拼接
            QString path = QString("%1/%2").arg(curPath).arg(fileName);

            // 判断是否为文件，封装PDU
            QFileInfo fileInfo(path);
            if(fileInfo.isFile())
            {
                // 是文件, 成员属性保存path
                m_downloadFilePath = path;
                PDU *respdu = mkPDU(FILE_TYPE_IS_FILE);
                respdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_START;

                // 封装文件大小
                qint64 size = fileInfo.size();
                sprintf(respdu->caData,"%lld",size);

                // 发送pdu
                this->write((char*)respdu,respdu->uiPDULen);

                // 设置定时器，timeout后 发送文件数据
                m_timer.start(2500);
                free(respdu);
                respdu = NULL;
            }
            else
            {
                // 不是文件
                PDU *respdu = mkPDU(FILE_TYPE_IS_DIR);
                respdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_START;
                // 发送pdu
                this->write((char*)respdu,respdu->uiPDULen);
                free(respdu);
                respdu = NULL;
            }

            delete []curPath;
            curPath = NULL;
            break;
        }
        case ENUM_MSG_TYPE_SHARE_REQUEST:
        {
            int num;
            char sendName[32] = {'\0'};
            sscanf(pdu->caData,"%s %d",sendName,&num);
            int size = num*32;
            qDebug() << num << " " << sendName;
            PDU *respdu = mkPDU(pdu->uiMsgLen-size);
            respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_NOTE;
            strncpy(respdu->caData,sendName,32);
            memcpy(respdu->caMsg,(char*)(pdu->caMsg)+size,pdu->uiMsgLen-size);
            char recvName[32] = {'\0'};
            for(int i=0;i<num;i++)
            {
                memcpy(recvName,(char*)(pdu->caMsg)+i*32,32);
                emit shareFileRelaySignal(recvName,respdu);
            }
            break;
        }
        case ENUM_MSG_TYPE_SHARE_NOTE_RESPOND:
        {
            char userName[32] = {'\0'};
            strncpy(userName,pdu->caData,32);
            QString destPath = Widget::getInstance().getUsrRootDirPath();
            destPath.append("/").append(userName);
            char *srcPath = new char[pdu->uiMsgLen];
            memcpy(srcPath,(char*)pdu->caMsg,pdu->uiMsgLen);
            //
            QString file = QString(srcPath);
            int index = file.lastIndexOf("/");
            file = file.last(file.size()-index);
            destPath.append(file);

            QFileInfo fileInfo(srcPath);
            if(fileInfo.isFile())
            {
                bool ret = QFile::copy(srcPath,destPath);
                qDebug() << " share file " << destPath << ret;
            }
            else if(fileInfo.isDir())
            {
                copyDir(srcPath,destPath);
                qDebug() << " share dir " << destPath;
            }
            else
            {
                qDebug() << " share failured ";
            }

            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_RESPOND;
            this->write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
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
        // 上传状态，直接接收原始QByteArray数据
        QByteArray buff = this->readAll();

        // 写入
        m_file.write(buff);
        m_Recved += buff.size();

        // 退出上传状态，并封装PDU
        if(m_Total <= m_Recved)
        {
            PDU *respdu = mkPDU(0);
            if(m_Total == m_Recved)
            {
                // 正常接收成功
                respdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND;
                strncpy(respdu->caData,UPLOAD_FILE_OK,strlen(UPLOAD_FILE_OK));
            }
            else
            {
                // 上传出现错误
                respdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND;
                strncpy(respdu->caData,UPLOAD_FILE_OK,strlen(UPLOAD_FILE_OK));
            }

            // 关闭file、更改标识、重置属性、发送PDU
            m_file.close();
            m_isUpload = false;
            m_Total = 0;
            m_Recved = 0;

            this->write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
        }
    }
}

void MyTcpSocket::clientOffline()
{
    OpenDB::getInstance().handleOffline(strName.toStdString().c_str());
    emit offline(this);
}

void MyTcpSocket::downloadingFile()
{
    QFile file(m_downloadFilePath);
    if(file.open(QIODevice::ReadOnly))
    {
        char *buffer = new char[4096];
        qint64 ret = 0;
        while(true)
        {
            ret = file.read(buffer,4096);
            if(ret > 0 && ret <= 4096)
            {
                this->write(buffer,4096);
            }
            else if(0 == ret)
            {
                break;
            }
            else
            {
                break;
            }
        }

        file.close();
        delete []buffer;
        buffer = NULL;
    }
}

void MyTcpSocket::copyDir(QString srcDir, QString destDir)
{
    QDir dir;
    dir.mkdir(destDir);
    dir.setPath(srcDir);
    QFileInfoList fileInfoList = dir.entryInfoList();
    QString srcTmp;
    QString destTmp;

    for(int i=0;i<fileInfoList.size();i++)
    {
        if(fileInfoList[i].fileName()=="." || fileInfoList[i].fileName()=="..")
        {
            continue;
        }

        if(fileInfoList[i].isFile())
        {
            srcTmp = srcDir+'/'
                     +fileInfoList[i].fileName();
            destTmp = destDir+'/'
                      +fileInfoList[i].fileName();
            QFile::copy(srcTmp,destTmp);
        }
        else if(fileInfoList[i].isDir())
        {
            srcTmp = srcDir+'/'
                     +fileInfoList[i].fileName();
            destTmp = destDir+'/'
                      +fileInfoList[i].fileName();
            // 递归
            copyDir(srcTmp,destTmp);
        }
    }
}
