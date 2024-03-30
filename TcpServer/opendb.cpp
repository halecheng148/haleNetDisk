#include "opendb.h"
#include <QDebug>
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

bool OpenDB::handleRegist(const char *name, const char *pwd)
{
    if(NULL == name || pwd == NULL)
    {
        return false;
    }
    QString sql = QString("insert into usrInfo(name,pwd) values('%1','%2')").arg(name).arg(pwd);
    QSqlQuery query;
    return query.exec(sql);
}

bool OpenDB::handleLogin(const char *name, const char *pwd)
{
    if(NULL == name || pwd == NULL)
    {
        return false;
    }
    QString sql = QString("select id from usrInfo where name=\'%1\' and pwd=\'%2\' and online=0").arg(name).arg(pwd);
    QSqlQuery query;
    query.exec(sql);
    bool ret;
    if(query.next())
    {
        sql = QString("update usrInfo set online=1 where id=%1").arg(query.value(0).toString().toInt());
        ret = query.exec(sql);
    }
    else
    {
        ret = false;
    }
    return ret;
}

void OpenDB::handleOffline(const char *name)
{
    if(NULL == name)
    {
        return;
    }
    QString sql = QString("update usrInfo set online=0 where name=\'%1\'").arg(name);
    QSqlQuery query;
    query.exec(sql);
}

QStringList OpenDB::handleAllOnline()
{
    QStringList list;
    list.clear();
    QString sql = QString("select name from usrInfo where online=1");
    QSqlQuery query;
    query.exec(sql);
    while(query.next())
    {
        list.append(query.value(0).toString());
    }

    return list;
}

int OpenDB::handleSearchUsr(const char *name)
{
    QString sql = QString("select online from usrInfo where name = \'%1\'").arg(name);
    QSqlQuery query;
    query.exec(sql);
    if(query.next())
    {
        return query.value(0).toInt();
    }
    return -1;
}

int OpenDB::handleAddFriend(QString friendName, QString myName)
{
    int friendId = getIdByName(friendName);
    if(friendId == -1)
        return 3;

    int myId = getIdByName(myName);

    QString sql = QString("select id from friend where id in(%1,%2) and friendId in(%3,%4)").arg(myId).arg(friendId).arg(myId).arg(friendId);

    QSqlQuery query;
    query.exec(sql);
    if(query.next())
    {
        // 双方已经是好友
        return 2;
    }
    if(isOnline(friendName))
    {
        return 1;
    }
    else
    {
        return 0;
    }

    return -1;

}

void OpenDB::handleInsertFriend(QString myName, QString friendName)
{
    int myId =  getIdByName(myName);
    int friendId = getIdByName(friendName);
    QString sql = QString("insert into friend(id,friendId) values(%1,%2)").arg(myId).arg(friendId);
    QSqlQuery query;
    query.exec(sql);
}

QStringList OpenDB::handleFlushFriend(const char *name)
{
    QStringList retList;
    retList.clear();
    if(name == NULL)
    {
        return retList;
    }
    int id = getIdByName(name);
    QString sql = QString("select name from usrInfo where online=1 and id in(select friendId from friend where id=%1)").arg(id);
    QSqlQuery query;
    query.exec(sql);
    while(query.next())
    {
        retList.append(query.value(0).toString());

        // qDebug() << query.value(0).toString();
    }
    query.clear();

    sql = QString("select name from usrInfo where online=1 and id in(select id from friend where friendId=%1)").arg(id);
    query.exec(sql);
    while(query.next())
    {
        retList.append(query.value(0).toString());
        // qDebug() << query.value(0).toString();
    }

    return retList;
}

void OpenDB::handleDelFriend(const char *myName, const char *friendName)
{
    int id = getIdByName(myName);
    int friendId = getIdByName(friendName);

    QString sql = QString("delete from friend where id in(%1,%2) and friendId in(%3,%4)").arg(id).arg(friendId).arg(id).arg(friendId);
    QSqlQuery query;
    query.exec(sql);
}

OpenDB &OpenDB::getInstance()
{
    static OpenDB instance;
    return instance;
}

int OpenDB::getIdByName(QString name)
{
    QString sql = QString("select id from usrInfo where name = \'%1\'").arg(name);
    QSqlQuery query;
    query.exec(sql);
    if(query.next())
    {
        return query.value(0).toInt();
    }
    return -1;
}

bool OpenDB::isOnline(QString name)
{
    bool ret = false;
    QString sql = QString("select online from usrInfo where name=\'%1\'").arg(name);
    QSqlQuery query;
    query.exec(sql);
    if(query.next())
    {
        if(query.value(0).toInt()==1)
            ret = true;
    }
    return ret;
}
