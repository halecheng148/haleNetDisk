#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define REGIST_OK "regist ok"
#define REGIST_FAILED "regist failed : name exsited"
#define LOGIN_OK "login ok"
#define LOGIN_FAILED "login failed : name err or pwd err or onlined"

#define SEARCH_USR_NO "no such people"
#define SEARCH_USR_ONLINE "online"
#define SEARCH_USR_OFFLINE "offline"

#define UNKNOW_ERROR "unknow error"
#define EXISTED_FRIEND "existed friend"
#define ADD_FRIEND_OFFLINE "usr offline"
#define ADD_FRIEND_NOEXIST "usr not exist"

#define ADD_FRIEDN_AGGREE "add friend success"
#define ADD_FRIEND_REFUSE "add friend failure"

#define DEL_FRIEND_OK "delete friend ok"

#define DIR_NO_EXSIT "dir not exist"
#define File_NAME_EXIST "file name exist"
#define CREATE_DIR_SUCCESS "create dir success"


#define DELETE_DIR_OK "delete dir ok"
#define DELETE_DIR_FAILURED "delete failed:is reguler file"

#define RENAME_FILE_OK "rename file ok"
#define RENAME_FILE_FAILURED "rename file failed"

#define UPLOAD_FILE_OK "upload file ok"
#define UPLOAD_FILE_FAIL "upload file fail"


enum ENUM_MSG_TYPE{
    ENUM_MSG_TYPE_MIN = 0,
    ENUM_MSG_TYPE_REGIST_REQUEST,
    ENUM_MSG_TYPE_REGIST_RESPOND,
    ENUM_MSG_TYPE_LOGIN_REQUEST,
    ENUM_MSG_TYPE_LOGIN_RESPOND,
    ENUM_MSG_TYPE_ALL_ONLINE_REQUEST,
    ENUM_MSG_TYPE_ALL_ONLINE_RESPOND,
    ENUM_MSG_TYPE_SEARCH_REQUEST,
    ENUM_MSG_TYPE_SEARCH_RESPOND,
    ENUM_MSG_TYPE_ADD_FRIEND_REQUEST,
    ENUM_MSG_TYPE_ADD_FRIEND_RESPOND,
    ENUM_MSG_TYPE_ADD_FRIEND_AGGREE,	// 添加好友同意
    ENUM_MSG_TYPE_ADD_FRIEND_REFUSE,	// 添加好友拒绝
    ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST, // 刷新好友列表请求
    ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND, // 刷新好友列表响应
    ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST, // 删除好友请求
    ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND, // 删除好友响应
    ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST, // 私聊请求
    ENUM_MSG_TYPE_PRIVATE_CHAT_RESPOND, // 私聊响应
    ENUM_MSG_TYPE_GROUP_CHAT_REQUEST,   // 群聊请求
    ENUM_MSG_TYPE_GROUP_CHAT_RESPOND,   // 群聊响应
    ENUM_MSG_TYPE_CREATE_DIR_REQUEST,   // 创建文件夹请求
    ENUM_MSG_TYPE_CREATE_DIR_RESPOND,   // 创建文件夹响应
    ENUM_MSG_TYPE_FLUSH_FILE_REQUEST,   // 显示当前路径所有文件请求
    ENUM_MSG_TYPE_FLUSH_FILE_RESPOND,    // 显示当前路径所有文件响应
    ENUM_MSG_TYPE_DELETE_DIR_REQUEST,   // 删除文件夹请求
    ENUM_MSG_TYPE_DELETE_DIR_RESPOND,   // 删除文件夹响应
    ENUM_MSG_TYPE_RENAME_FILE_REQUEST,  // 文件重命名请求
    ENUM_MSG_TYPE_RENAME_FILE_RESPOND,  // 文件重命名响应
    ENUM_MSG_TYPE_ENTER_DIR_REQUEST,    // 进入文件夹请求
    ENUM_MSG_TYPE_ENTER_DIR_RESPOND,    // 进入文件夹响应
    ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST,  // 上传文件请求
    ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND,  // 上传文件响应
    ENUM_MSG_TYPE_DELETE_FILE_REQUEST,  // 删除文件请求
    ENUM_MSG_TYPE_DELETE_FILE_RESPOND,  // 删除文件响应
    ENUM_MSG_TYPE_DELETE_FILE_ISDIR,    // 服务器回应文件是一个目录
    ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST, // 下载文件请求
    ENUM_MSG_TYPE_DOWNLOAD_FILE_START,   // 服务器开始传输数据信号
    ENUM_MSG_TYPE_SHARE_REQUEST,
    ENUM_MSG_TYPE_SHARE_RESPOND,
    ENUM_MSG_TYPE_SHARE_NOTE, // 通知有人共享文件给你
    ENUM_MSG_TYPE_SHARE_NOTE_RESPOND,
    ENUM_MSG_TYPE_MAX = 0x00fffff
};

enum FILE_TYPE{
    FILE_TYPE_IS_DIR = 0,
    FILE_TYPE_IS_FILE = 1
};

typedef unsigned int uint;
typedef struct PDU
{
    uint uiPDULen;
    uint uiMsgType;
    char caData[64];
    uint uiMsgLen;
    int caMsg[];
}PDU;

PDU *mkPDU(uint uiMsgLen);


typedef struct FileInfo
{
    char fileName[32];
    int fileType;
}FileInfo;

#endif // PROTOCOL_H
