#ifndef OPENDB_H
#define OPENDB_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
class OpenDB : public QObject
{
    Q_OBJECT
public:
    OpenDB();
    ~OpenDB();

    void init();

    /**
     * 注册信息录入
     * @brief handleRegist
     * @param name
     * @param pwd
     * @return
     */
    bool handleRegist(const char *name,const char *pwd);

    /**
     * 登录查询及状态修改
     * @brief handleLogin
     * @param name
     * @param pwd
     * @return
     */
    bool handleLogin(const char *name,const char *pwd);

    /**
     * 登录状态修改
     * @brief handleOffline
     * @param name
     */
    void handleOffline(const char *name);

    /**
     * 返回所有在线用户名称
     * @brief handleAllOnline
     * @return
     */
    QStringList handleAllOnline();

    /**
     * 查询用户是否存在，以及是否在线
     * @brief handleSearchUsr
     * @param name
     * @return -1:no such people、1:online、0:offline
     */
    int handleSearchUsr(const char* name);


    /**
     * 查询friend表中是否存在(friendId,myId)或（myId,friendId)
     * 其中一个存在就代表两者是好友
     * 如果不是好友，则还需查询对方是否存在
     * 如果对方存在，还需查询对方是否在线
     * @brief handleAddFriend
     * @param friendName
     * @param myName
     * @return -1:添加失败,未知原因|2:双方已经是好友|0：对方不在线|1：对方在线|3：对方不存在
     */
    int handleAddFriend(QString friendName,QString myName);

    /**
     * 将好友信息添加进入数据表friend中
     * @brief handleInsertFriend
     * @param myName
     * @param friendName
     */
    void handleInsertFriend(QString myName,QString friendName);

    /**
     * 查询name 的所有在线好友
     * @brief handleFlushFriend
     * @param name
     * @return 返回在线好友的nameList
     */
    QStringList handleFlushFriend(const char *name);

    /**
     * 删除好友列表中的一条好友数据
     * @brief handleDelFriend
     * @param myName
     * @param friendName
     */
    void handleDelFriend(const char *myName,const char *friendName);


    static OpenDB &getInstance();

private:
    /**
     * 方便friend表的查询
     * @brief getIdByName
     * @param name
     * @return 返回name的id号，-1表示失败
     */
    int getIdByName(QString name);

    bool isOnline(QString name);

private:
    QSqlDatabase m_database;
};

#endif // OPENDB_H
