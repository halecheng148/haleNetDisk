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
    pTcpSocket->setSocketDescriptor(socketDescrptor);
    m_tcpSocketList.append(pTcpSocket);
}
```

