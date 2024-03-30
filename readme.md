### 一、数据库设计

#### 1、设计数据库结构

```sqlite
# 创建数据库文件
sqlite3 cloud.db
# 创建用户表
create table usrInfo(id integer primary key autoincrement,
                    name varchar(32),
                    pwd varchar(32));
                    
# 创建好友表
create table friendInfo(id integer not null,
                       friendId integer not null,
                       primary key(id,friendId));
```



#### 2、插入数据

```
insert into usrInfo(name,pwd) 
values('admin','admin'),('cheng','cheng');
```



### 二、客户端服务器的搭建

#### 1、配置文件

- 将服务器IP和PORT信息填入配置文件中
- 将配置文件作为资源文件添加到资源文件中
- 程序运行时加载配置文件中的数据

> Qt creator > new project > Qt widget Application  > name=TcpClient>qmake>widget
>
> 文件名 ： client.config
>
> 写入信息
>
> ​	127.0.0.1
>
> ​	8888
>
> 添加资源文件
>
>  
>
> 添加属性：
>
> QString m_strIP;
>
> quint16 m_usPort;

```c++
QString m_strIP;                // IP地址
quint16 m_usPort;               // 端口号
```

```c++
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
        qDebug() << m_strIP << " " << m_usPort;

    }
    else{
        QMessageBox::critical(this,"config open","config open err");
    }
}
```



####2、客户端实现

![image-20240325164838005](D:\Qt_C++_NetDiskSystem\project_start_designer_images\image-20240325164838005.png)

- 加入QT 的网络编程模块 network

- 异步链接服务器   TcpSocket

  - ```
       QTcpSocket m_socket;
       	// 连接服务器,异步 
        m_socket.connectToHost(QHostAddress(m_strIP),m_usPort);
    ```

- 绑定服务器连接成功信号

  - ```
    void Widget::showConnected()
    {
        QMessageBox::information(this,"连接服务器","连接服务器成功");
    }
    
    // 连接成功弹窗
    connect(&m_socket,SIGNAL(connected()),this,SLOT(showConnected()));
    ```

  

####3、服务器端监听连接

- 加入配置文件，写入监听ip和port
- 读取配置文件，
- 加入QT网络模块 network
- 单例模式饿汉式返回QTcpServer的实例

```
MyTcpServer &MyTcpServer::getInstance()
{
    static MyTcpServer instance;
    return instance;
}
```

- 进行监听

```
// 监听
MyTcpServer::getInstance().listen(QHostAddress(m_strIP),m_usPort);
```



- incomingConnection(qintptr socket...) 重写

```
// 当有新的连接可用时，QTcpServer调用这个虚函数。
// socketDescriptor参数是已接受连接的本机套接字描述符。
void MyTcpServer::incomingConnection(qintptr socketDescriptor)
{
    qDebug() << "new client connected";
}
```



### 三、通讯协议

#### 1、弹性结构体

```c++
typedef struct PDU
{
    int a;
    int b;
    int c;
    int d[];		// 大小不确定
}PDU;

int main(int argc;char *argv[])
{
    printf("%ld\n",sizeof(PDU));		// 输出 3*sizeof(int)
    // 弹性结构体分配空间
    PDU *pdu = (PDU*)malloc(sizeof(PDU)+100*sizeof(int));
    pdu->a = 90;
    pdu->b = 89;
    pdu->c = 88;
    memcpy(pdu->d,"you jump i jump",16);
    printf("a=%d,b=%d,c=%d,%s\n",pdu->a,pdu->b,pdu->c,
          (char*)(pdu->d));
    
    free(pdu);
    pdu = null;
    return 0;
}
```



#### 2、通讯协议设计

![image-20240325184311164](.\project_start_designer_images\image-20240325184311164.png)

protocol:

![image-20240325184712740](.\project_start_designer_images\image-20240325184712740.png)

![image-20240325185002280](.\project_start_designer_images\image-20240325185002280.png)



#### 3、数据收发测试

- 服务器端接收：

```c++
// 创建MyTcpSocket
MyTcpSocket::MyTcpSocket()...
{
	connect(this,SIGNAL(readyRead())
			,this,SLOT(recvMsg()));
}

void MyTcpSocket::recvMsg()
{
    // 返回可用于读取的字节数。
    qDebug() << this->bytesAvailable();
    uint uiPDULen = 0;
    // 读取总协议单元长度
    this->read((char*)&uiPDULen,sizeof(uint));
    
    // 实际的消息长度，弹性结构体
   	uint uiMsgLen = uiPDULen-sizeof(PDU);
    // 创建一个PDU
    PDU *pdu = mkPDU(uiMsgLen);
    // 读取消息,因为已经接收了一个数据,减去它
    this->read((char*)pdu+sizeof(uint),uiPDULen-sizeof(uint));
    
}


// 在MyTcpServer中
private:
	QList<MyTcpSocket*> m_tcpSocketList;

incomingConnection(方法中)
{
    // 。。。
    MyTcpSocket *pTcpSocket = new MyTcpSocket;
    // 为socket设置描述符，相当于绑定一个数据来源
    pTcpSocket->setSocketDescriptor(socketDescrptor);
    // 将socket放入列表中
    m_tcpSocketList.append(pTcpSocket);
}
```

-  客户端 发送信息

```c++

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

```



### 四、登录注册注销

#### 1、数据库操作

（1）定义数据库操作类

（2）将数据库操作类定义成单例

（3）数据库操作

- usrInfo表

  | id     | integer     | 主键、自增                  |
  | ------ | ----------- | --------------------------- |
  | name   | varchar(32) | unique唯一                  |
  | pwd    | varchar(32) |                             |
  | online | integer     | default 0 、 0=离线，1=在线 |

- friend

| id       | integer |      |
| -------- | ------- | ---- |
| friendId | integer |      |

tip:   `primary key(id,friendId)`



- OpenDB.h、OpenDB.cpp
  - 继承QObject
  - 单例设计模式创建获取实例，饿汉式
  - 加入数据库模块 QT +=sql
  - QSqlDataBase连接数据库

```c++

private:
    QSqlDatabase m_database;

// .cpp中
OpenDB::OpenDB() {
    m_database = QSqlDatabase::addDatabase("QSQLITE");
}

OpenDB::~OpenDB()
{
    m_database.close();
}

void OpenDB::init()
{
    m_database.setHostName("localhost");
    m_database.setDatabaseName("D:\\Qt_C++_NetDiskSystem\\TcpServer\\cloud.db");
    if(m_database.open())
    {
        QSqlQuery query;
        query.exec("select * from usrInfo");
        while(query.next())
        {
            QString data = QString("%1,%2,%3").arg(query.value(0).toString()).arg(query.value(1).toString()).arg(query.value(2).toString());
            qDebug() << data;
        }
    }

}

OpenDB &OpenDB::getInstance()
{
    static OpenDB instance;
    return instance;
}

```



#### 2、登录注册注销请求

- 消息类型设计

```
#defind REGIST_OK "regist ok"
#defind REGIST_FAILED "regist failed : name existed"
enum ENUM_MSG_TYPE
{
	ENUM_MSG_TYPE_MIN=0,
	ENUM_MSG_TYPE_REGIST_REQUEST,		// 注册请求
	ENUM_MSG_TYPE_REGIST_RESPOND,		// 注册回复
	ENUM_MSG_TYPE_MAX=0x00ffffff
};
```



- 界面设计

![image-20240325225452779](.\project_start_designer_images\image-20240325225452779.png)

![image-20240325225527657](.\project_start_designer_images\image-20240325225527657.png)

echoMode:Password

- 注册，用户名唯一，防止重复注册

![](.\project_start_designer_images\SequenceDiagram1.png)

TcpClient--widget：发送注册请求

```c++
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
```

mytcpserver:重写incomingConnection(qintptr socketDescirptor),监听接收socket描述符

```c++
// .h中
private:
    QList<MyTcpSocket*> m_tcpSocketList;

//.cpp中
void MyTcpServer::incomingConnection(qintptr socketDescriptor)
{
    qDebug() << "new client connected";

    MyTcpSocket *pTcpSocket = new MyTcpSocket;
    pTcpSocket->setSocketDescriptor(socketDescriptor);
    m_tcpSocketList.append(pTcpSocket);
}
```

TcpServer MyTcpSocket

```c++
// 在构造函数中
connect(this,SIGNAL(readyRead()),this,SLOT(recvMsg()));

// 接收请求
void MyTcpSocket::recvMsg()
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
        }else{
            strcpy(respdu->caData,REGIST_FAILED);
        }

        this->write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    default:
        break;
    }
}
```

TcpClient widget

```c++
// 接收响应
void Widget::recvMsg()
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
    default:
        break;
    }

    free(pdu);
    pdu = NULL;
}
```



- 登录，防止重复登录

```
#define LOGIN_OK "login ok"
#define LOGIN_FAILED "login failed : name err or pwd err o relogin"
ENUM_MSG_TYPE_LOGIN_REQUEST
ENUM_MSG_TYPE_LOGIN_RESPOND

1、控件事件发送登录请求
2、服务器socket中的recvMsg方法 扩展登录请求的处理方法，并最后返回响应信息
3、OPenDB,添加bool handleLogin(name,pwd)方法
	online=0 ==> 成功：update table set online=1 where name=\'%1\' and pwd...
4、客户端recvMsg处理响应信息


1、客户端退出，服务器端删除对应socket，修改对应用户的online
2、自定义slots clientOffline() ,socket自带 signal disconnected()
这是服务器端的。
3、void handleOffline(const char* name); 处理数据库online
4、发送信号 offline(MyTcpSocket*),
5、TcpServer::slots deleteSocket(MyTcpSocket*) 接收处理，迭代遍历
m_tcpSocketList,删除mysocket==*item,
```

- 注销，删除好友信息，删除个人信息，删除网盘文件

之后；

#### 3、登录注册注销回复



### 五、好友操作

#### 1、界面设计

friend.ui

![image-20240326143101887](.\project_start_designer_images\image-20240326143101887.png)

book.ui

...// 之后

OpeWidget.ui

![image-20240326152904602](.\project_start_designer_images\image-20240326152904602.png)

#### 2、登录跳转

```
1、将OpeWidget设置成为单例模式
2、登录成功：将OpeWidget显示出来 .show()
3、隐藏登录界面 .hide()
```

#### 3、查看在线用户

![image-20240326154323733](.\project_start_designer_images\image-20240326154323733.png)

```
1、将TcpClient Widget 单例设计
2、编写Widget::getTcpSocket
3、online.h 中 引用 pdu,注意重复引用问题
4、修改pdu
ENUM_MSG_TYPE_ALL_ONLINE_REQUEST,
ENUM_MSG_TYPE_ALL_ONLINE_RESPOND,
ENUM_MSG_TYPE_SEARCH_REQUEST,
ENUM_MSG_TYPE_SEARCH_RESPOND,
5、void Friend::showHideOnlineWidget() 下编写请求


服务器端
1、recvMsg扩写
2、QStringList OpenDB::handleAllOnline(); 
	select name from usrInfo where online=1;
3、计算MsgLen = ret.size()*32;
memcpy((char*)(respdu->caMsg)+i*32,ret.at(i).tostdString().c_str,ret.at(i).size());
4、发送封装好的respdu


客户端
1、接收，将pdu中的nameList逐层传递到online.cpp
2、读取pdu 将信息显示
```

#### 4、搜索用户

![image-20240326154435067](.\project_start_designer_images\image-20240326154435067.png)

```
客户端
friend: 
1、查询按键clicked信号绑架槽函数，
添加一个记录name的属性m_searchName;
2、QString name = QInputDialog::getText(this,"搜索","用户名:");
3、pdu打包 mkPDU(0) ENUM_MSG_TYPE_SEARCH_USR_REQUEST
4、发送请求 


protocol:
#define SEARCH_USR_NO "no such people"
#define SEARCH_USR_ONLINE "online"
#define SEARCH_USR_OFFLINE "offline"

服务器
1、int OpenDB::handleSearchUsr(cosnt char* name);
select online from usrInfo where name = '%1'
-1:no such people
1:online
0:offline
2、respdu响应打包 0 ENUM_MSG_TYPE_SEARCH_USR_RESPOND
 strcpy(respdu->caData,"online|offline|...")
3、发送响应信息

客户端
widget:
case ...RESPOND:
1、解析读取PDU
2、向用户显示操作响应的信息 
```



#### 5、添加好友

![image-20240326201341115](.\project_start_designer_images\image-20240326201341115.png)

```
protocol
ENUM_MSG_TYPE_ADD_FRIEND_REQUEST,
ENUM_MSG_TYPE_ADD_FRIEND_RESPOND,

ENUM_MSG_TYPE_ADD_FRIEND_AGGREE,	// 同意
ENUM_MSG_TYPE_ADD_FRIEND_REFUSE		// 拒绝 

#define UNKNOW_ERROR "unknow error"
#define EXISTED_FRIEND "existed friend"
#define ADD_FRIEND_OFFLINE "usr offline";
#define ADD_FRIEND_NOEXIST "usr not exist"

#define ADD_FRIEDN_AGGREE "add friend agree"
#define ADD_FRIEND_REFUSE "add friend refuse"

客户端A
1、点击加好友按钮，编辑槽函数Online::on_add_Friend_clicked()
2、获取online_lw中的当前选中的item 的文字（用户名）
3、修改登录时，需要用一个成员变量记录当前登录的用户名
4、获取登录的用户名
5、pdu mkPDU(0) 
ENUM_MSG_TYPE_ADD_FRIEND_REQUEST
memcpy(pdu->caData,选中的用户名,32);
memcpy(pdu->caData+32,登录的用户名,32);
6、数据发送


服务器
1、int OpenDB::handleAddFriend(const char *pername,const char *name);
-1:添加失败,未知原因
2:双方已经是好友
0：对方不在线
1：对方在线
3：对方不存在

2、根据handleAddFriend的返回值 ，做出判断
3、ret==-1: 
respdu = mkPDU(0)
respdu->uiMsgType = ...
strcpy(respdu->caData,UNKNOW_ERROR);
4、ret == 2:
respdu = mkPDU(0)
respdu->uiMsgType = ...
strcpy(respdu->caData,EXISTED_FRIEND);
5、ret == 0:
respdu = mkPDU(0)
respdu->uiMsgType = ...
strcpy(respdu->caData,ADD_FRIEND_OFFLINE);
6、ret == 3
respdu = mkPDU(0)
respdu->uiMsgType = ...
strcpy(respdu->caData,ADD_FRIEND_NOEXIST);

7、ret == 1: 对方在线，需要转发,直接转发客户端A的pdu
	在tcpserver中编写转发请求的函数，因为socketlist所在
		void resend(const char *parname,PDU* pdu);
	遍历得到对方的socket，
	发送pdu
	

客户端接收两种
ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:// 是否同意对方添加
1、提取对方名字 strmcpy(caName,pdu->caData+32,32);
2、QMessageBox::informatino(this,"添加好友",QString("%1 的好友申请").arg(caName),QMessageBox::Yes,QMessageBox::No);
3、写回复respdu，respdu.caData = pdu->caData
	ENUM_MSG_TYPE_ADD_FRIEND_AGGREE // 同意
	ENUM_MSG_TYPE_ADD_FRIEND_REFUSE // 拒绝
4、 发送respdu

ENUM_MSG_TYPE_ADD_FRIEND_RESPOND: // 添加失败或已经添加
QMessageBox::information(this,"添加好友",pdu->caData);



服务器
ENUM_MSG_TYPE_ADD_FRIEND_AGGREE
通知双方成为好友 使用TcpServer 遍历socket 转发
将信息加入到数据库
ENUM_MSG_TYPE_ADD_FRIEND_REFUSE
通知客户端A添加失败 使用TcpServer 遍历socket 转发
```



#### 6、刷新好友列表

![image-20240327142359853](.\project_start_designer_images\image-20240327142359853.png)

```
protocol:
ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST,
ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND,

客户端：
1、刷新按钮，clicked信号绑定槽函数slots flushFriend()
2、获取登录用户名
3、封装PDU，memcpy(pdu->caData,name.toStdString().c_str(),name.size());
ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST
4、发送请求


服务器
1、数据库操作QStringList handleFlushFriend(const char *name)
	select name from usrInfo where online=1 and id=(select friendId from friend where id in(select id from usrInfo where name='%1')).arg(name);
	select name from usrInfo where online=1 and id in(select id from friend where friendId=(select id from usrInfo where name='%1')).arg(name);
	封装QStringList,并返回
2、获取用户名,调用数据库函数
	char name[32] = {'\0'};
	strncpy(name,pdu->caData,32);
	QStringList ret = 数据库函数；
3、封装PDU，
uint uiMsgLen = ret.size()*32;
mkPDU(uiMsgLen)
循环拷贝
	memcpy((char*)(respdu->caMsg)+i*32
			,ret.at(i).toStdString().c_str()
			,ret.at(i).size());
4、发送响应


客户端
1、计算个数  uiMsgLen/32
2、char name[32]...,循环memcpy
3、m_pFriendListWidget.addItem
```





#### 7、删除好友

![image-20240327162459703](.\project_start_designer_images\image-20240327162459703.png)

```
protocol
ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST,
ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND,

#define DEL_FRIEND_OK "delete friend ok"

客户端
1、为删除按钮的clicked信号绑定一个slots delFriend()
2、选择要删除的好友，注意判空
if(NULL!=m_pFriendListWidget.current...())
3、获取自己的名字（登录的名字）
4、封装PDU
mkPDU(0)
ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST
memcpy(pdu->caData,myName,size());
memcpy(pdu.caData+32,friendName,size());
5、发送删除请求


服务器
1、数据库 void handleDelFriend(const char *myName,const char* friendName);

2、获取登录用户名 ，要删除的好友名
3、调用数据库函数
4、通知双方
删除的好友不在线，就仅通知登录用户
ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND，
strcpy(respdu->caData,DEL_FRIEND_OK);
发送respdu
同时使用TcpServer转发pdu到friendName (if online)



客户端
ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST：
	获取对方名字，提示对方将自己删除
ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND：
	提示删除成功
```





#### 8、私聊

![image-20240327175938012](.\project_start_designer_images\image-20240327175938012.png)

(1) 设计界面

![image-20240327180138142](.\project_start_designer_images\image-20240327180138142.png)

![image-20240327180207492](.\project_start_designer_images\image-20240327180207492.png)

设置布局 以及 字体大小



```
在PrivateChat.h
include "potocol.h"

记录聊天双方的名字 myName,friendName;
提供setMyName(...)、setFriendName(...)
```



(2) 实现私聊

```
potocol:
ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST,
ENUM_MSG_TYPE_PRIVATE_CHAT_RESPOND,


客户端
privateChat
1、发送按钮clicked信号绑定槽函数
2、获取输入框聊天内容，提示不能为空
3、封装PDU
mkPDU(inputMsg.size()+1)
memcpy(pdu->caDate
		,myName.toStdString().c_str()
		,myName.size())
memcpy(pdu->caDate+32
		,friendName.toStdString().c_str()
		,friendName.size())
strcpy((char*)(pdu->caMsg)
		,inputMsg.toStdString().c_str())
4、发送私聊请求

friend
5、私聊按钮clicked信息绑定槽函数（用于显示私聊窗口，并赋双方名的值）
6、m_pPrivateChatWidget属性,创建
7、显示



服务器端
1、获取friendName
2、发出privateChatSignal(name,pdu);
3、TcpServer中绑定转发槽函数



客户端
privateChat
如果私聊窗口是隐藏的，则显示，同时设置聊天对象名
设置在TextEidt中显示聊天内容的updateMsg(const PDU *pdu)
谁 says: 啥
```



#### 9、群聊

![image-20240327233919943](D:\Qt_C++_NetDiskSystem\project_start_designer_images\image-20240327233919943.png)

```
protocol
ENUM_MSG_TYPE_GROUP_CHAT_REQUEST,
ENUM_MSG_TYPE_GROUP_CHAT_RESPOND,

客户端
1、提供群聊发送按钮的槽函数
2、获取群聊输入框的文本
3、封装PDU
mkPDU(strMsg.size()+1)
ENUM_MSG_TYPE_GROUP_CHAT_REQUEST
strncpy(pdu->caData
		,myName.toStdString().c_str()
		,myName.size());
strncpy((char*)(pdu->caMsg)
		,strMsg.to...
		,strMsg.size());
4、发送



服务器
1、提取发送者的用户名
2、获取发送者所有在线的好友的nameList
	OpenDB::handleFlushFriend
3、循环遍历nameList,并转发pdu


客户端
1、读取pdu
2、将信息以 "friendName said:  caMsg"形式，显示在群聊TextEdit上
```





### 六、文件操作

#### 1、界面设计

```c++
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
```

![image-20240328001038162](D:\Qt_C++_NetDiskSystem\project_start_designer_images\image-20240328001038162.png)

#### 2、创建文件夹

![image-20240328134721444](.\project_start_designer_images\image-20240328134721444.png)

```

根目录自动创建
服务器
1、每个用户注册时，都会创建一个以"用户名"命名的 根目录
	bool QDir::mkDir(QString("./%1").arg(caName))
	
	
客户端
1、添加一个属性用于记录当前文件夹
2、登录后进入根目录



子目录创建
protocol
ENUM_MSG_TYPE_CREATE_DIR_REQUEST,
ENUM_MSG_TYPE_CREATE_DIR_RESPOND,

#define DIR_NO_EXSIT "dir not exist"
#define File_NAME_EXIST "file name exist"
#define CREATE_DIR_SUCCESS "create dir success"
客户端
创建按钮绑定槽函数
1、获取用户名、当前目录、创建文件夹的名称
2、QInputDialog::getText(this,"","") 获取新文件夹名称,newDir的名字不能超过32个字符
3、封装PDU
mkPDU(curPath.size()+1)
strncpy(pdu->caData,myName,32);
strncpy(pdu->caData+32,newDir,32);
memcpy(pdu->caMsg,curPath,curPath.size())
4、发送


服务器端
1、获取创建文件夹的目录地址 curPath,以及newDir
2、判断是否存在
（1）当前目录存在 exists(curPath)
	- 判断newDir是否重名，通过
		exists(curPath+"/"+newDIr)判断
	- 不重名就创建
	- 将错误信息或创建成功信息封装进respdu中
（2）当前目录不存在
	- 将错误信息封装进入respdu中
3、发送



客户端
读取pdu 
使用弹窗将信息显示
```





#### 3、查看所有文件

- 查看当前目录下的所有子目录及文件

![image-20240328160306625](D:\Qt_C++_NetDiskSystem\project_start_designer_images\image-20240328160306625.png)

- 服务器要返回文件的名称和类型信息

```
protocal
ENUM_MSG_TYPE_FLUSH_FILE_REQUEST,
ENUM_MSG_TYPE_FLUSH_FILE_RESPOND,

struct FileInfo
{
	char fileName[32];
	int fileType
};

enum FILE_TYPE
{
	FILE_TYPE_IS_DIR = 0;
	FILE_TYPE_IS_FILE = 1;
};

客户端
1、刷新按钮绑定槽函数
2、获取当前目录 curPath
3、封装PDU
mkPDU(curPath.size()+1)
ENUM_MSG_TYPE_FLUSH_FILE_REQUEST
strncpy((char*)pdu->caMsg,curPath,size())
4、发送


服务器
1、
char *pCurPath = new char[pdu->uiMsgLen];
memcpy(curPath,pdu->caMsg,pdu->uiMsgLen);
2、获取当前目录下所有文件信息
QFileInfoList QDir::entryInfoList(curPath)
3、封装respdu
int fileCount = fileInfoList.size()-2;
mkPDU(sizeof(FileInfo)*fileCount);
ENUM_MSG_TYPE_FLUSH_FILE_RESPOND
FileInfo *pFileInfo = NULL;
QString strFileName;
for(int i=0 ; i< fileCount+2;i++)
{
	// 排除"." 和 ".."
	pFileInfo = (FileInfo*)(respdu->caMsg)+i;
	strFileName = fileInfoList[i].fileName();
	memcpy(pFileInfo->fileName
			,strFileName..
			,size());
	if(fileInfoList[i].isDir())
	{
		pFileInfo->fileType = 0;
	}
	else if(fileInfoList[i].isFile())
	{
		pFileInfo->fileType = 1;
	}
}
4、发送


客户端准备
1、文件夹图标、文件图标
2、添加图标到资源文件中
客户端
1、widget接收PDU , 传给book
2、解析PDU,并显示
int fileCount = pdu->uiMsgLen/sizeof(FileInfo);
FileInfo *pFileInfo = NULL;
for(int i=0 ; i< fileCount;i++)
{
	pFileInfo = (FileInfo*)(pdu->caMsg)+i;
	QListWidgetItem *pItem = new QListWidgetItem;
	pItem->setIcon(QIcon(QPixmap(":/map/dir.jpg")))
	// or
	pItem->setIcon(QIcon(QPixmap(":/map/reg.jpg")))
	pItem->setText(pFileInfo->fileName);
	ui->pBookListW->addItem(pItem);
}
```





#### 4、删除文件夹

![image-20240328224731209](.\project_start_designer_images\image-20240328224731209.png)

```
protocol
ENUM_MSG_TYPE_DELETE_DIR_REQUEST,
ENUM_MSG_TYPE_DELETE_DIR_RESPOND,

#define DELETE_DIR_OK "delete dir ok"
#define DELETE_DIR_FAILURED "delete failed:is reguler file"

客户端
1、为删除文件夹按钮绑定槽函数
2、获取当前目录路径
3、获取选中的文件夹 QListWidget::currentItem();
4、封装pdu
mkPDU(0)
ENUM_MSG_TYPE_DELETE_DIR_REQUEST
strncpy(caData,strDelName,..)
memcpy(caMsg,strCurPath,..)
5、发送


服务器
1、解析pdu,获取caData中的文件名
2、获取caMsg中的路径名
char *pPath = new char[pdu->uiMsgLen];
3、拼接strPath = curPath+"/"+delName
4、判断delName是否为目录 
QFileInfo fileInfo(strPath);
fileInfo.isDir();
	(1) 是文件夹
		QDir dir(strPath);
		ret=dir.removeRecursively(); // 递归删除
	(2) 是文件
		ret=false; // 这里是删除文件夹
5、封装respdu,发送


客户端
解析pdu,弹窗显示caData信息
```





#### 5、文件重命名

![image-20240328231308777](.\project_start_designer_images\image-20240328231308777.png)

```
protocol
ENUM_MSG_TYPE_RENAME_FILE_REQUEST,
ENUM_MSG_TYPE_RENAME_FILE_RESPOND,
#define RENAME_FILE_OK "rename file ok"
#define RENAME_FILE_FAILURED "rename file failed"
客户端
1、为相关按钮绑定槽函数
2、获取当前所在目录
3、获取OldName,
QListWidgetItem *item = m_pBookListW->currentItem();
item!=NULL;
4、获取NewName,使用QInputDialog::getText()
5、两个名字放进caData，路径放入caMsg
6、发送



服务器
1、解析pdu,获取oldName,newName,path
2、拼接两条路径
oldPath = path+oldName
newPath = path+newName
3、重命名
QDir dir;
ret = dir.rename(oldPath,newPath);
4、根据ret ,封装respdu,
信息放入caData中
5、发送



客户端
1、解析pdu
2、弹窗显示caData
```







#### 6、进入文件夹

![image-20240328232859874](.\project_start_designer_images\image-20240328232859874.png)

```
protocol
ENUM_MSG_TYPE_ENTER_DIR_REQUEST,
ENUM_MSG_TYPE_ENTER_DIR_RESPOND,



客户端
1、为QListWidget::doubleClicked(const QModelIndex index)绑定槽函数
	enterDir(const QModelIndex index)
2、获取文件名dirName，通过 QModelIndex::data()
3、获取当前路径，curPath
4、封装pdu
curPath 放入 caMsg
dirName 放入 caData中
ENUM_MSG_TYPE_ENTER_DIR_REQUEST
5、发送



服务器
1、获取dirName,curPath
2、拼接
path = curPath + "/" +dirName
3、判断是否为路径
QFileInfo fileInfo(path);
fileInfo.isDir()
	(1) 是文件夹
		path放入caMsg
		FILE_TYPE_IS_DIR
		发送respdu
	(2) 不是文件夹
		FILE_TYPE_IS_FILE
		发送respdu
		

客户端
根据结果，选择是否刷新,不刷新则弹窗显示信息
Widget::getInstance.setCurrentPath(QString curPath)
onFlushFilePBClicked()
```





#### 7、返回上一级

![image-20240328235718194](.\project_start_designer_images\image-20240328235718194.png)

```
判断当前是不是rootDir
主要是修改Widget::m_curPath
然后刷新即可

将当前路径去掉最后一个
int index = curPath.lastIndexOf('/');
curPath.remove(index,curPath.size()-index);
```





#### 8、上传文件

![image-20240329180628381](.\project_start_designer_images\image-20240329180628381.png)

```
m_iRecved)
{
	m_file.close();
	m_bUpload = false;
	
	// 创建PDU,告诉
}protocol
ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST,
ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND,

#define UPLOAD_FILE_OK "upload file ok"
#define UPLOAD_FILE_FAIL "upload file fail"

客户端
1、上传按钮槽函数
2、获取当前路径、以及将要上传的文件路径
使用QFileDialog::getOpenFileName();
3、判断上传路径不能为空
4、提取文件名，
int index = filePath.lastIndexOf("/");
QString fileName = filePath.right(filePath.size()-index-1);
5、获取文件大小
QFile file(filePath)
qint64 fileSize = file.size();
6、封装PDU
mkPDU(curPath.size()+1)
sprintf(pdu->data,"%s %lld",fileName.toStrString().c_str(),fileSize);
7、发送
8、设置一个定时器，在定时结束后直接上传文件
QTimer::timeout() 信号 绑定 uploadingHandle():



服务器
1、获取文件名、文件大小、当前目录路径
char fileName[32] = {'\0'};
	sscanf(pdu->caData,"%s %lld"
			,fileName,&fileSize);
2、创建成员属性
QFile m_file;
qint64 m_iTotal; // 文件总大小
qint64 m_iRecved; // 已经读取的大小
bool m_bUpload;	// 是否为上传文件状态

// 在socket构造函数中初始化m_bUpload=false

3、拼接路径,以只写的方式打开文件
path = curPath+"/"+fileName;
m_file.setFileName(path);
// 没有文件，将自动创建文件
if(m_file.open(QIODevice::WriteOnly))
{
	m_bUpload = true;
	m_iTotal = fileSize;
	m_iRecved = 0;
}

4、recvMsg()方法中，判断当前是否为上传文件
if(!m_bUpload)
{
	// 原来的代码
	switch(pdu..){.......}
}
else
{
	// 上传文件内容代码，不是请求处理
}




客户端
uploadingHandle():
1、获取要上传的文件路径（成员变量记录）
2、只读打开
3、创建一个缓存区
char *buffer = new char[4096];
qint64 ret = 0;
while(true)
{
	ret = file.read(buffer,4096);
	if(ret>0 && ret <=4096)
	{
		Widget::getInstance.write(buffer,4096);
	}
	else if(0==ret)
	{
		break;
	}
	else
	{
		QMessageBox::warning(this,"","读取文件失败");
		break;
	}
}
file.close();
delete []buffer;
buffer = NULL;




服务器
1、使用QByteArray buff = QTcpSocket::readAll()读取文件
2、
m_file.write(buff);
m_iRecved += buff.size();
if(m_iTotal <= m_iRecved)
{
	if(==){
		m_file.close();
		m_bUpload = false;
		// 创建PDU,告诉客户端上传成功
	}
	else if(<)
	{
		m_file.close();
		m_bUpload = false;
		// 创建PDU,告诉客户端上传失败
	}
}



客户端
解析pdu,弹窗显示
```







#### 9、删除常规文件

![image-20240329190920190](.\project_start_designer_images\image-20240329190920190.png)

```
protocol
ENUM_MSG_TYPE_DELETE_FILE_REQUEST,  // 删除文件请求
ENUM_MSG_TYPE_DELETE_FILE_RESPOND,  // 删除文件响应
ENUM_MSG_TYPE_DELETE_FILE_ISDIR,
QDir dir;
dir.remove(filePath);
```





#### 10、下载文件

![image-20240329191113088](.\project_start_designer_images\image-20240329191113088.png)

```
protocol
ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST,
ENUM_MSG_TYPE_DOWNLOAD_FILE_START,

// 指定保存路径
QString savePath = QFileDialog::getSaveFileName();
//将这个路径保存在成员变量中

// 设置一个bool变量，标记是否正在下载，初始false
// 服务器回复后设置为true




// 服务器打开文件，
// 封装文件大小，和文件名，发送给客户端

// 客户端记录文件大小，以及创建打空白文件，标识设置为true

// 服务器，设置缓存区，并发送文件数据


// 客户端读取保存数据
```

tip :上传文件相反



#### 11、分享文件

![image-20240329193117065](.\project_start_designer_images\image-20240329193117065.png)

![image-20240329193801852](.\project_start_designer_images\image-20240329193801852.png)

![image-20240329193838408](.\project_start_designer_images\image-20240329193838408.png)

![image-20240329194408284](.\project_start_designer_images\image-20240329194408284.png)

![image-20240329194207379](D:\Qt_C++_NetDiskSystem\project_start_designer_images\image-20240329194207379.png)

```
m_pButtonGroup->setExclusive(false) // 多选
```



```
获取所有在线的好友，显示在sharefile窗口中中
sharefile单例设计

实现全选和取消选择
QList<QAbstractButton *> QButtonGroup::buttons() 
QAbstractButton::
	bool isChecked() 
	void setChecked(bool)
	

确定分享
ENUM_MSG_TYPE_SHARE_REQUEST,
ENUM_MSG_TYPE_SHARE_RESPOND,
ENUM_MSG_TYPE_SHARE_NOTE, // 通知有人共享文件给你
ENUM_MSG_TYPE_SHARE_NOTE_RESPOND,
客户端
book保存共享文件路径，提供接口获取
获取用户名，共享文件路径、选中的好友list
封装PDU
mkPDU(32*friendNum+sharePath.size()+1)
sprintf(pdu->caData,"%s %d"
		,usrName.toStdString().c_str()
		,friendNum);

for()
{
	memcpy((char*)(pdu->caMsg)+i*32
			,friendList[i].toStdString().c_str()
			,32);
}
memcpy((char*)(pdu->caMsg)+friendNum*32
		,sharePath.toStdString().c_str()
		,sharePath.size());
发送pdu


服务器
获取分享者名字、分享人数
int num;char sendName[32]={'\0'};
sscanf(pdu->caData,"%s%d",sendName,&num)
int size = num*32;
封装PDU
respdu = mkPDU(pdu->uiMsgLen-size);
ENUM_MSG_TYPE_SHARE_NOTE
strcpy(respdu->caData,sendName);
memcpy(respdu->caMsg
		,(char*)(pdu->caMsg)+size
		,pdu->uiMsgLen-size); // 共享的路径

char recvName[32] = {'\0'};
for()
{
	memcpy(recvName
		,(char*)(pdu->caMsg)+i*32
		,32);
	// 转发
	handleRelay(recvName,respdu);
}
给分享者回复信息，
分享者的客户端读取弹窗显示信息



被分享者的客户端
1、提取分享路径、分享者的名字
2、解析文件名 通过lasdIndexOf()....
3、int ret = QMessageBox::questing(this,"",谁分享了啥)
4、根据ret封装PDU,ret=false时不发送了
ENUM_MSG_TYPE_SHARE_NOTE_RESPOND
caMsg 放 分享路径
caData 放 当前用户名
5、发送

服务器
获取分享文件路径、用户名,解析fileName
服务器rootDir+"/"+用户名+fileName，得到用户根目录usrPath
QFileInfo fileInfo(shareFilePath);
if(fileInfo.isFile())
{
	QFile::copy(shareFilePath,usrPath);
}
else if(fileInfo.isDir())
{
 	copyDir(shareFilePath,usrPath)
}
void copyDir(QString srcDir,QString destDir)
{
	QDir dir;
	dir.mkDir(destDir);
	dir.setPath(srcDir);
	QFileInfoList fileInfoList = dir.entryInfoList();
	QString srcTmp;
	QString destTmp;
	
	for(inr i=0;i<fileInfoList.size();i++)
	{
		if(fileInfoList[i].fileName=="." || 
			fileInfoList[i].fileName=="..")
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

取消分享
隐藏sharefile窗口
```







#### 12、移动文件

![image-20240329234447462](D:\Qt_C++_NetDiskSystem\project_start_designer_images\image-20240329234447462.png)

```
protocol:
ENUM_MSG_TYPE_MOVE_FILE_REQUEST,
ENUM_MSG_TYPE_MOVE_FILE_RESPOND,



客户端
1、添加一个移动文件按钮在book中 *m_moveFilePB
2、创建槽函数
3、m_pBookListW->currentItem() ,获取当前文件名moveFileName
4、获取当前路径，与文件名拼接moveFilePath
5、使用成员变量保存m_moveFilePath,m_moveFileName
7、目标目录按钮设置为可用
8、创建一个目标目录按钮，*m_SelectDirPB (成员变量);只有需要时可用
9、获取当前的目录为目标目录，
10、将目标目录保存在m_destDir 成员变量中
11、封装PDU
int srcLen = m_moveFilePath.size()
int destLen = m_destDir.size()
mkPDU(srcLen+destLen+2)
ENUM_MSG_TYPE_MOVE_FILE_REQUEST
sprintf(pdu->caData,"%d%d%s",srcLen,destLen
		,m_moveFileName.toStdString().c_str())
memcpy((char*)pdu->caMsg
		,m_moveFilePath..toStdString().c_str()
		,srcLen)
memcpy((char*)(pdu->caMsg)+(srcLen+1)
		,m_destDir..toStdString().c_str()
		,destLen)
12、发送



服务器
char fileName[32]
int srcLen
int destLen
sscanf(pdu->caData,,"%d%d%s",&srcLen,&destLen,fileName)
char *srcPath = new char[srcLen+1];
char *destPath = new char[destLen+1+32];
memset(srcPath,'\0',srcLen+1)
memset(destPath,'\0',destLen+1+32)
memcpy(srcPath,(char*)pdu->caMsg,srcLen);
memcpy(destPath,(char*)(pdu->caMsg)+(srcLen+1),destLen)

QFileInfo fileInfo(destPath);
if(fileInfo.isDir())
{
	// 可以移动到这
	strcat(destPath,"/");
	strcat(destPath,fileName); // 拼接目标文件
	// 同路径下重命名，不同路径下移动
	bool ret = QFile::rename(srcPath,destPath);
	if(ret)
	{
		// 封装respdu,成功信息
	}
	else
	{
		// 封装respdu,失败信息
	}
}
else if(fileInfo.isFile())
{
	// 移动失败。这里不是目录
	// 封装respdu,失败信息
}


客户端
读取pdu,弹窗显示信息
```

