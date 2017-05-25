#include "mysqloperation.h"
#include <QDateTime>
#include <QDebug>

QString MysqlOperation::MysqlAddr;
QString MysqlOperation::MysqlDatabaseName;
QString MysqlOperation::MysqlPort;
QString MysqlOperation::MysqlUserName;
QString MysqlOperation::MysqlPassword;

QSqlDatabase MysqlOperation::m_database;
bool MysqlOperation::m_dbIsconnect = false;

MysqlOperation::MysqlOperation()
{

}

bool MysqlOperation::openMysqlDatabase()
{
    //打开数据库
    m_database = QSqlDatabase::addDatabase("QMYSQL");
    m_database.setHostName(MysqlAddr);  //设置主机地址
    m_database.setPort(MysqlPort.toInt(NULL, 10));  //设置端口
    m_database.setDatabaseName(MysqlDatabaseName);  //设置数据库名称
    m_database.setUserName(MysqlUserName);  //设置用户名
    m_database.setPassword(MysqlPassword);  //设置密码
    if(!m_database.open())
    {
        qDebug()<<"failed to connect to mysql:" << m_database.lastError();
        m_dbIsconnect = false;
        return false;
    }
    else
    {
        qDebug()<<"success to connect to mysql";
        m_dbIsconnect = true;
        return true;
    }

}

void MysqlOperation::closeMysqlDatabase()
{
    qDebug() << QObject::tr("数据库已经关闭");
    m_database.close();
    m_dbIsconnect = false;
}

bool MysqlOperation::createAdminTable()
{
//  0编号 1姓名 2登录名 3密码 4所属权限组 5所属用户组 6联系方式 7权限状态
//  8权限起始日期 9权限截止日期
    QString createTableStr = "create table if not exists adminTable"
                             "(id integer auto_increment, "
                             "callName varchar(20) not null, "
                             "logName varchar(20) not null, "
                             "password varchar(20) not null, "
                             "powerGroup varchar(20) not null default '操作员', "
                             "userGroup varchar(50), "
                             "phoneNum varchar(26) default null, "
                             "empower varchar(20) not null default '未授权', "
                             "empowerST timestamp default current_timestamp, "
                             "empowerET timestamp default current_timestamp, "
                             "primary key(id), "
                             "unique(logName), "
                             "unique(callName))";

    QSqlQuery sqlQuery;
    bool ok = sqlQuery.exec(createTableStr);
    if(ok)
    {
        qDebug() << QObject::tr("创建数据库表adminTable成功");
        return true;
    }
    else
    {
        qDebug() << QObject::tr("创建数据库表adminTable失败:") << m_database.lastError();
        return false;
    }
}

int MysqlOperation::isAdminInDatabase(QString loginName, QString loginPassword)
{
    QString format("select * from adminTable "
                   "where logName='%1' and password='%2'");
    QString queryStr = format.arg(loginName).arg(loginPassword);
    qDebug() << "queryStr = "<<queryStr;

    QSqlQuery sqlQuery;
    bool ok = sqlQuery.exec(queryStr);
    if(ok)
    {
        if(sqlQuery.next())
        {
            qDebug() << QObject::tr("该用户存在");
            return 1;
        }
        else
        {
            qDebug() << QObject::tr("该用户不存在");

            return 0;
        }
    }
    else
    {
        qDebug() << QObject::tr("查询数据库失败:") << m_database.lastError();

        return -1;
    }
}

int MysqlOperation::isAdminEmpower(QString loginName, QString loginPassword)
{
    QString format("select empower from adminTable "
                   "where logName='%1' and password='%2'");
    QString queryStr = format.arg(loginName).arg(loginPassword);
    qDebug() << "queryStr = "<<queryStr;

    QSqlQuery sqlQuery;
    bool ok = sqlQuery.exec(queryStr);
    if(ok)
    {
        if(sqlQuery.next())
        {
            if(sqlQuery.value(0).toString() == QObject::tr("未授权"))
            {
                qDebug() << QObject::tr("该用户未授权");
                return 0;
            }
            else if(sqlQuery.value(0).toString() == QObject::tr("权限过期"))
            {
                qDebug() << QObject::tr("该用户权限过期");
                return -2;
            }
            else if(sqlQuery.value(0).toString() == QObject::tr("权限未到"))
            {
                qDebug() << QObject::tr("该用户权限未到");
                return -3;
            }
            else if(sqlQuery.value(0).toString() == QObject::tr("拉入黑名单"))
            {
                qDebug() << QObject::tr("该用户已被拉入黑名单");
                return -4;
            }
            else if(sqlQuery.value(0).toString() == QObject::tr("期间授权"))
            {
                qDebug() << QObject::tr("该用户已期间授权");
                return 1;
            }
            else
            {
                qDebug() << QObject::tr("该用户已永久授权");
                return 2;
            }

        }
        else
        {
            return 0;
        }
    }
    else
    {
        qDebug() << QObject::tr("查询数据库失败:") << m_database.lastError();

        return -1;
    }
}

int MysqlOperation::isAdminRoot(QString loginName, QString loginPassword)
{
    QString format("select powerGroup from adminTable "
                   "where logName='%1' and password='%2'");
    QString queryStr = format.arg(loginName).arg(loginPassword);
    qDebug() << "queryStr = "<<queryStr;

    QSqlQuery sqlQuery;
    bool ok = sqlQuery.exec(queryStr);
    if(ok)
    {
        if(sqlQuery.next())
        {
            if(sqlQuery.value(0).toString() == QObject::tr("操作员"))
            {
                qDebug() << QObject::tr("该用户为操作员");
                return 0;
            }
            else
            {
                qDebug() << QObject::tr("该用户为管理员");
                return 1;
            }

        }
        else
        {
            return 0;
        }
    }
    else
    {
        qDebug() << QObject::tr("查询数据库失败:") << m_database.lastError();

        return -1;
    }
}

int MysqlOperation::isAdminRootBylogName(QString logName)
{
    QString format("select powerGroup from adminTable where logName='%1'");
    QString queryStr = format.arg(logName);
    qDebug() << "queryStr = "<<queryStr;

    QSqlQuery sqlQuery;
    bool ok = sqlQuery.exec(queryStr);
    if(ok)
    {
        if(sqlQuery.next())
        {
            if(sqlQuery.value(0).toString() == QObject::tr("操作员"))
            {
                qDebug() << QObject::tr("该用户为操作员");
                return 0;
            }
            else
            {
                qDebug() << QObject::tr("该用户为管理员");
                return 1;
            }

        }
        else
        {
            return 0;
        }
    }
    else
    {
        qDebug() << QObject::tr("查询数据库失败:") << m_database.lastError();

        return -1;
    }
}

bool MysqlOperation::createLockTable()
{
//      0编号 1锁编号 2锁名称 3所属用户组 4地理位置 5经度 6纬度 7fsuIP地址 8锁用户 9服役时间 10锁IP
        QString createTableStr = "create table if not exists lockTable"
                                 "(id integer auto_increment, "
                                 "lockCode varchar(30), "
                                 "lockName varchar(30), "
                                 "userGroup varchar(50), "
                                 "lockAddr varchar(50), "
                                 "jingDu varchar(15), "
                                 "weiDu varchar(15), "
                                 "ipAddr varchar(30) not null, "
                                 "lockUser varchar(128) not null default 'admin,555888;', "
                                 "startTime timestamp default current_timestamp, "
                                 "lockIP varchar(30) default '200.192.128.1', "
                                 "primary key(id), "
                                 "unique(lockCode),"
                                 "unique(ipAddr))";
        QSqlQuery sqlQuery;
        bool ok = sqlQuery.exec(createTableStr);
        if(ok)
        {
            qDebug() << QObject::tr("创建数据库表lockTable成功");
            return true;
        }
        else
        {
            qDebug() << QObject::tr("创建数据库表lockTable失败:") << m_database.lastError();
            return false;
        }
}

bool MysqlOperation::createKeyTable()
{
//    0编号 1钥匙编号 2钥匙名称 3所属用户组 4A级密钥 5B级密钥 6权限状态 7权限掩码 8权限起始日期 9权限截止日期 10keyIP
    QString createTableStr = "create table if not exists keyTable"
                             "(id integer auto_increment, "
                             "keyCode varchar(20), "
                             "keyName varchar(30), "
                             "userGroup varchar(50), "
                             "ska varchar(20), "
                             "skb varchar(20), "
                             "empower varchar(20), "
                             "mc varchar(20), "
                             "startTime varchar(20), "
                             "endTime varchar(20), "
                             "keyIP varchar(30), "
                             "primary key(id), "
                             "unique(keyIP))";
    QSqlQuery sqlQuery;
    bool ok = sqlQuery.exec(createTableStr);
    if(ok)
    {
        qDebug() << QObject::tr("创建数据库表keyTable成功");
        return true;
    }
    else
    {
        qDebug() << QObject::tr("创建数据库表keyTable失败:") << m_database.lastError();
        return false;
    }
}

int MysqlOperation::iskeyIpInDatabase(QString keyIp)
{
    QString format("select * from keyTable where keyIP='%1'");
    QString queryStr = format.arg(keyIp);
    qDebug() << "queryStr = "<<queryStr;

    QSqlQuery sqlQuery;
    bool ok = sqlQuery.exec(queryStr);
    if(ok)
    {
        if(sqlQuery.next())
        {
            qDebug() << QObject::tr("该钥匙IP存在");
            return 1;
        }
        else
        {
            qDebug() << QObject::tr("该钥匙IP不存在");

            return 0;
        }
    }
    else
    {
        qDebug() << QObject::tr("查询数据库失败:") << m_database.lastError();

        return -1;
    }
}

bool MysqlOperation::insertIntoKeyTable(QVector<QString> keyInfoVector)
{
    QString format("insert into keyTable(keyCode,keyName,userGroup,"
                   "ska,skb,empower,mc,startTime,endTime,keyIP) "
                   "values('%1','%2','%3','%4','%5','%6','%7','%8','%9','%10')");
    QString queryStr = format.arg(keyInfoVector.at(0))
            .arg(keyInfoVector.at(1))
            .arg(keyInfoVector.at(2))
            .arg(keyInfoVector.at(3))
            .arg(keyInfoVector.at(4))
            .arg(keyInfoVector.at(5))
            .arg(keyInfoVector.at(6))
            .arg(keyInfoVector.at(7))
            .arg(keyInfoVector.at(8))
            .arg(keyInfoVector.at(9));
    qDebug() << "queryStr = "<<queryStr;

    QSqlQuery sqlQuery;
    bool ok = sqlQuery.exec(queryStr);
    if(ok)
    {
        qDebug() << QObject::tr("添加新钥匙成功！");
        return true;
    }
    else
    {
        qDebug() << QObject::tr("添加新钥匙失败:") << m_database.lastError();
        return false;
    }
}

bool MysqlOperation::updateSKA(QString keyIP, QString skaStr)
{
    QString format("update keyTable set ska='%1' where keyIP='%2'");
    QString queryStr = format.arg(skaStr).arg(keyIP);
    qDebug() << "queryStr = "<<queryStr;

    QSqlQuery sqlQuery;
    bool ok = sqlQuery.exec(queryStr);
    if(ok)
    {
        qDebug() << QObject::tr("更新钥匙SKA成功！");
        return true;
    }
    else
    {
        qDebug() << QObject::tr("更新钥匙SKA失败:") << m_database.lastError();
        return false;
    }
}

bool MysqlOperation::updateSKB(QString keyIP, QString skbStr)
{
    QString format("update keyTable set skb='%1' where keyIP='%2'");
    QString queryStr = format.arg(skbStr).arg(keyIP);
    qDebug() << "queryStr = "<<queryStr;

    QSqlQuery sqlQuery;
    bool ok = sqlQuery.exec(queryStr);
    if(ok)
    {
        qDebug() << QObject::tr("更新钥匙SKB成功！");
        return true;
    }
    else
    {
        qDebug() << QObject::tr("更新钥匙SKB失败:") << m_database.lastError();
        return false;
    }
}



bool MysqlOperation::updateEmpower(QString keyIP,QString empower,
                                   QString mc,QString startTime,QString endTime)
{
    QString format("update keyTable set empower='%1', mc='%2', "
                   "startTime='%3', endTime='%4' where keyIP='%5'");
    QString queryStr = format.arg(empower).arg(mc).arg(startTime).arg(endTime).arg(keyIP);
    qDebug() << "queryStr = "<<queryStr;

    QSqlQuery sqlQuery;
    bool ok = sqlQuery.exec(queryStr);
    if(ok)
    {
        qDebug() << QObject::tr("更新钥匙权限成功！");
        return true;
    }
    else
    {
        qDebug() << QObject::tr("更新钥匙权限失败:") << m_database.lastError();
        return false;
    }
}

 bool MysqlOperation::updateKeyIP(QString oldKeyIP,QString newKeyIP)
 {
     QString format("update keyTable set keyIP='%1' where keyIP='%2'");
     QString queryStr = format.arg(newKeyIP).arg(oldKeyIP);
     qDebug() << "queryStr = "<<queryStr;

     QSqlQuery sqlQuery;
     bool ok = sqlQuery.exec(queryStr);
     if(ok)
     {
         qDebug() << QObject::tr("更新钥匙编码成功！");
         return true;
     }
     else
     {
         qDebug() << QObject::tr("更新钥匙编码失败:") << m_database.lastError();
         return false;
     }
 }

 bool MysqlOperation::updateLockIP(QString lockCode,QString lockIP)
 {
     QString format("update lockTable set lockIP='%1' where lockCode='%2'");
     QString queryStr = format.arg(lockIP).arg(lockCode);
     qDebug() << "queryStr = "<<queryStr;

     QSqlQuery sqlQuery;
     bool ok = sqlQuery.exec(queryStr);
     if(ok)
     {
         qDebug() << QObject::tr("更新锁ID成功！");
         return true;
     }
     else
     {
         qDebug() << QObject::tr("更新锁ID失败:") << m_database.lastError();
         return false;
     }
 }

QString MysqlOperation::getLockCodeByIPv4(QString IPv4String)
{
      QSqlQuery sqlQuery;
      QString format("select lockCode from lockTable where ipAddr='%1'");
      QString queryStr = format.arg(IPv4String);
      qDebug() << "queryStr = "<<queryStr;
      if(sqlQuery.exec(queryStr))
      {
          if(sqlQuery.next())
          {
              return sqlQuery.value(0).toString();
          }
          else
          {
              qDebug() << QObject::tr("没有查找到与IP相应的lockCode");
          }

      }
      else
      {
           qDebug() << QObject::tr("查询数据库失败:") << m_database.lastError();
      }

      return NULL;
}

QString MysqlOperation::getLockIpBylockCode(QString lockCode)
{
    QSqlQuery sqlQuery;
    QString format("select lockIP from lockTable where lockCode='%1'");
    QString queryStr = format.arg(lockCode);
    qDebug() << "queryStr = "<<queryStr;
    if(sqlQuery.exec(queryStr))
    {
        if(sqlQuery.next())
        {
            return sqlQuery.value(0).toString();
        }
        else
        {
            qDebug() << QObject::tr("没有查找到与lockCode相应的lockIP");
        }

    }
    else
    {
         qDebug() << QObject::tr("查询数据库失败:") << m_database.lastError();
    }

    return NULL;
}

QString MysqlOperation::getLockCodeByLnglat(QString jingDu,QString weiDu)
{
    QSqlQuery sqlQuery;
    QString format("select lockCode from lockTable where jingDu='%1' and weiDu='%2'");
    QString queryStr = format.arg(jingDu).arg(weiDu);
    qDebug() << "queryStr = "<<queryStr;
    if(sqlQuery.exec(queryStr))
    {
        if(sqlQuery.next())
        {
            return sqlQuery.value(0).toString();
        }
        else
        {
            qDebug() << QObject::tr("没有查找到与经纬度相应的lockCode");
        }

    }
    else
    {
         qDebug() << QObject::tr("查询数据库失败:") << m_database.lastError();
    }

    return NULL;
}

bool MysqlOperation::isLockIPTheSame(QString lockCode,QString lockIP)
{
    QSqlQuery sqlQuery;
    QString format("select lockIP from lockTable where lockCode='%1'");
    QString queryStr = format.arg(lockCode);
    qDebug() << "queryStr = "<<queryStr;
    if(sqlQuery.exec(queryStr))
    {
        if(sqlQuery.next())
        {
            if(sqlQuery.value(0).toString() == lockIP)
            {
                return true;
            }
            else
            {
                return false;
            }

        }
        else
        {
            qDebug() << QObject::tr("编号为%1的锁具未存在于数据库中").arg(lockCode);
        }

    }
    else
    {
         qDebug() << QObject::tr("查询数据库失败:") << m_database.lastError();
    }

    return true;
}

QString MysqlOperation::getSkaBykeyIP(QString keyIP)
{
    QSqlQuery sqlQuery;
    QString format("select ska from keyTable where keyIP='%1'");
    QString queryStr = format.arg(keyIP);
    qDebug() << "queryStr = "<<queryStr;
    if(sqlQuery.exec(queryStr))
    {
        if(sqlQuery.next())
        {
            return sqlQuery.value(0).toString();
        }
        else
        {
            qDebug() << QObject::tr("没有查找到与keyIP相应的ska");
        }

    }
    else
    {
         qDebug() << QObject::tr("查询数据库失败:") << m_database.lastError();
    }

    return NULL;
}

QString MysqlOperation::getkeyCodeBykeyIP(QString keyIP)
{
    QSqlQuery sqlQuery;
    QString format("select keyCode from keyTable where keyIP='%1'");
    QString queryStr = format.arg(keyIP);
    qDebug() << "queryStr = "<<queryStr;
    if(sqlQuery.exec(queryStr))
    {
        if(sqlQuery.next())
        {
            return sqlQuery.value(0).toString();
        }
        else
        {
            qDebug() << QObject::tr("没有查找到与keyIP相应的keyCode");
        }

    }
    else
    {
         qDebug() << QObject::tr("查询数据库失败:") << m_database.lastError();
    }

    return NULL;
}

void MysqlOperation::getLockNameAddrGroupByCode(QString lockCode,QString&lockName,
                                                QString&lockAddr,QString&userGroup)
{
    QSqlQuery sqlQuery;
    QString format("select lockName,userGroup,lockAddr from lockTable where lockCode='%1'");
    QString queryStr = format.arg(lockCode);
    qDebug() << "queryStr = "<<queryStr;
    if(sqlQuery.exec(queryStr))
    {
        if(sqlQuery.next())
        {
            lockName = sqlQuery.value(0).toString();
            userGroup = sqlQuery.value(1).toString();
            lockAddr = sqlQuery.value(2).toString();
        }
    }
    else
    {
         qDebug() << QObject::tr("查询数据库失败:") << m_database.lastError();
    }
}

void MysqlOperation::getLockAddrGroupByCode(QString lockCode,QString&lockAddr,QString&userGroup)
{
    QSqlQuery sqlQuery;
    QString format("select userGroup,lockAddr from lockTable where lockCode='%1'");
    QString queryStr = format.arg(lockCode);
    qDebug() << "queryStr = "<<queryStr;
    if(sqlQuery.exec(queryStr))
    {
        if(sqlQuery.next())
        {
            userGroup = sqlQuery.value(0).toString();
            lockAddr = sqlQuery.value(1).toString();
        }
    }
    else
    {
         qDebug() << QObject::tr("查询数据库失败:") << m_database.lastError();
    }
}

void MysqlOperation::getCallNameBylogName(QString logName,QString&callName,QString&phoneNum)
{
    QSqlQuery sqlQuery;
    QString format("select callName,phoneNum from adminTable where logName='%1'");
    QString queryStr = format.arg(logName);
    qDebug() << "queryStr = "<<queryStr;
    if(sqlQuery.exec(queryStr))
    {
        if(sqlQuery.next())
        {
            callName = sqlQuery.value(0).toString();
            phoneNum = sqlQuery.value(1).toString();
        }
    }
    else
    {
         qDebug() << QObject::tr("查询数据库失败:") << m_database.lastError();
    }
}

void MysqlOperation::getStEtBylogName(QString logName,QString&sTime,QString&eTime)
{
    QSqlQuery sqlQuery;
    QString format("select empowerST,empowerET from adminTable where logName='%1'");
    QString queryStr = format.arg(logName);
    qDebug() << "queryStr = "<<queryStr;
    if(sqlQuery.exec(queryStr))
    {
        if(sqlQuery.next())
        {
            sTime = sqlQuery.value(0).toString();
            eTime = sqlQuery.value(1).toString();
        }
    }
    else
    {
        sTime = "";
        eTime = "";
         qDebug() << QObject::tr("查询数据库失败:") << m_database.lastError();
    }
}

bool MysqlOperation::getAdminInfoByLogName(QString logName,QVector<QString>& adminInfoVector)
{
    QSqlQuery sqlQuery;
    QString format("select callName,logName,password,powerGroup,userGroup,"
                   "phoneNum,empower,empowerST,empowerET"
                   " from adminTable where logName='%1'");
    QString queryStr = format.arg(logName);
    qDebug() << "queryStr = "<<queryStr;
    if(sqlQuery.exec(queryStr))
    {
        if(sqlQuery.next())
        {
            adminInfoVector.append(sqlQuery.value(0).toString());
            adminInfoVector.append(sqlQuery.value(1).toString());
            adminInfoVector.append(sqlQuery.value(2).toString());
            adminInfoVector.append(sqlQuery.value(3).toString());
            adminInfoVector.append(sqlQuery.value(4).toString());
            adminInfoVector.append(sqlQuery.value(5).toString());
            adminInfoVector.append(sqlQuery.value(6).toString());
            adminInfoVector.append(sqlQuery.value(7).toDateTime().toString("yyyy/MM/dd hh:mm"));
            adminInfoVector.append(sqlQuery.value(8).toDateTime().toString("yyyy/MM/dd hh:mm"));

            return true;
        }
        else
        {
            adminInfoVector.append("");
            adminInfoVector.append("");
            adminInfoVector.append("");
            adminInfoVector.append("");
            adminInfoVector.append("");
            adminInfoVector.append("");
            adminInfoVector.append("");
            adminInfoVector.append("");
            adminInfoVector.append("");
        }
    }
    else
    {
        qDebug() << QObject::tr("查询数据库失败:") << m_database.lastError();
        adminInfoVector.append("");
        adminInfoVector.append("");
        adminInfoVector.append("");
        adminInfoVector.append("");
        adminInfoVector.append("");
        adminInfoVector.append("");
        adminInfoVector.append("");
        adminInfoVector.append("");
        adminInfoVector.append("");
    }

    return false;
}


bool MysqlOperation::updateAdminInfoByLogName(QString logName,QVector<QString> adminInfoVector)
{
    QSqlQuery sqlQuery;
    QString format("update adminTable set callName='%1',logName='%2',password='%3',"
                   "powerGroup='%4',userGroup='%5',"
                   "phoneNum='%6',empower='%7',empowerST='%8',empowerET='%9'"
                   " where logName='%10'");
    QString queryStr = format.arg(adminInfoVector.at(0)).arg(adminInfoVector.at(1))
            .arg(adminInfoVector.at(2)).arg(adminInfoVector.at(3)).arg(adminInfoVector.at(4))
            .arg(adminInfoVector.at(5)).arg(adminInfoVector.at(6)).arg(adminInfoVector.at(7))
            .arg(adminInfoVector.at(8)).arg(logName);
    qDebug() << "queryStr = "<<queryStr;
    if(sqlQuery.exec(queryStr))
    {
        return true;
    }
    else
    {
        qDebug() << QObject::tr("查询数据库失败:") << m_database.lastError();
    }

    return false;
}

void MysqlOperation::updateEmpower()
{
    QString queryStr = "select id,empower,empowerST,empowerET from adminTable";
    qDebug() << "queryStr = "<<queryStr;

    QSqlQuery sqlQuery;
    bool ok = sqlQuery.exec(queryStr);
    if(ok)
    {
        while(sqlQuery.next())
        {
            if(!(sqlQuery.value(1).toString() == QObject::tr("永久授权")) &&
                    !(sqlQuery.value(1).toString() == QObject::tr("未授权"))
                    && !(sqlQuery.value(1).toString() == QObject::tr("列入黑名单")))
            {
                QDateTime time = QDateTime::currentDateTime();
                if(sqlQuery.value(2).toDateTime() > time)
                {
                    QString format("update adminTable set empower='%1' where id=%2");
                    QString queryStr = format.arg(QObject::tr("权限未到")).arg(sqlQuery.value(0).toString());
                    qDebug() << "queryStr = "<<queryStr;
                    QSqlQuery sqlQuery;
                    bool ok = sqlQuery.exec(queryStr);
                    if(ok)
                    {
                        qDebug() << QObject::tr("更新数据库成功");
                    }
                    else
                    {
                        qDebug() << QObject::tr("更新数据库失败:") << m_database.lastError();
                        return;
                    }
                }

                if(sqlQuery.value(3).toDateTime() < time)
                {
                    QString format("update adminTable set empower='%1' where id=%2");
                    QString queryStr = format.arg(QObject::tr("权限已过期")).arg(sqlQuery.value(0).toString());
                    qDebug() << "queryStr = "<< queryStr;
                    QSqlQuery sqlQuery;
                    bool ok = sqlQuery.exec(queryStr);
                    if(ok)
                    {
                        qDebug() << QObject::tr("更新数据库成功");
                    }
                    else
                    {
                        qDebug() << QObject::tr("更新数据库失败:") << m_database.lastError();
                        return;
                    }
                }
            }
        }
    }
    else
    {
        qDebug() << QObject::tr("查询数据库失败:") << m_database.lastError();
        return;
    }
}

bool MysqlOperation::createRecordTable()
{
    //    0编号 1姓名  2编号 3日志类型 4事件内容 5发生时间
        QString createTableStr = "create table if not exists recordTable"
                                 "(id integer auto_increment, "
                                 "callName varchar(20), "
                                 "devCode varchar(20), "
                                 "recordType varchar(20), "
                                 "eventCtx varchar(128), "
                                 "happenT timestamp default current_timestamp, "
                                 "primary key(id))";
        QSqlQuery sqlQuery;
        bool ok = sqlQuery.exec(createTableStr);
        if(ok)
        {
            qDebug() << QObject::tr("创建数据库表recordTable成功");
            return true;
        }
        else
        {
            qDebug() << QObject::tr("创建数据库表recordTable失败:") << m_database.lastError();
            return false;
        }
}

bool MysqlOperation::createGroupTable()
{
//        0编号 1用户组 2创建时间
        QString createTableStr = "create table if not exists groupTable"
                                 "(id integer auto_increment, "
                                 "groupName varchar(50), "
                                 "startTime timestamp default current_timestamp, "
                                 "primary key(id), "
                                 "unique(groupName))";
        QSqlQuery sqlQuery;
        bool ok = sqlQuery.exec(createTableStr);
        if(ok)
        {
            qDebug() << QObject::tr("创建数据库表groupTable成功");
            return true;
        }
        else
        {
            qDebug() << QObject::tr("创建数据库表groupTable失败:") << m_database.lastError();
            return false;
        }
}

void MysqlOperation::insertRecord(QString callName,QString devCode,QString type,QString content)
{

    QSqlQuery sqlQuery;
    QString format("insert into recordTable(callName,devCode,recordType,eventCtx) "
                   "values('%1','%2','%3','%4')");
    QString queryStr = format.arg(callName).arg(devCode).arg(type).arg(content);
    qDebug() << "queryStr = "<<queryStr;
    if(sqlQuery.exec(queryStr))
    {
        qDebug() << QObject::tr("插入record成功");
    }
    else
    {
        qDebug() << QObject::tr("插入record失败");
    }
}

void MysqlOperation::getLockInfo(QVector<QString>& lockCodeVector,
                                   QVector<QString>& lockNameVector,
                                   QVector<QString>& lockAddrVector,
                                 QVector<QString>&lockJingDuVector,
                                 QVector<QString>&lockWeiDuVector)
{
    QSqlQuery sqlQuery;
    QString queryStr = "select lockCode,lockName,lockAddr,jingDu,weiDu from lockTable";
    qDebug() << "queryStr = "<<queryStr;
    if(sqlQuery.exec(queryStr))
    {
        while(sqlQuery.next())
        {
            lockCodeVector.append(sqlQuery.value(0).toString());
            lockNameVector.append(sqlQuery.value(1).toString());
            lockAddrVector.append(sqlQuery.value(2).toString());
            lockJingDuVector.append(sqlQuery.value(3).toString());
            lockWeiDuVector.append(sqlQuery.value(4).toString());
        }
    }
    else
    {
         qDebug() << QObject::tr("查询数据库失败:") << m_database.lastError();
         lockCodeVector.append("");
         lockNameVector.append("");
         lockAddrVector.append("");
         lockJingDuVector.append("");
         lockWeiDuVector.append("");
    }
}

void MysqlOperation::getLockInfoForMap(QVector<QString>&lockCodeVector,
                                       QVector<QString>&lockNameVector,
                                       QVector<QString>&userGroupVector,
                                       QVector<QString>&lockAddrVector,
                                       QVector<QString>&jingDuVector,
                                       QVector<QString>&weiDuVector,
                                       QVector<QString>&ipAddrVector,
                                       QVector<QString>&startTimeVector)
{
    QSqlQuery sqlQuery;
    QString queryStr = "select lockCode,lockName,userGroup,lockAddr,jingDu,weiDu,"
                       "ipAddr,startTime from lockTable";
    qDebug() << "queryStr = "<<queryStr;
    if(sqlQuery.exec(queryStr))
    {
        while(sqlQuery.next())
        {
            lockCodeVector.append(sqlQuery.value(0).toString());
            lockNameVector.append(sqlQuery.value(1).toString());
            userGroupVector.append(sqlQuery.value(2).toString());
            lockAddrVector.append(sqlQuery.value(3).toString());
            jingDuVector.append(sqlQuery.value(4).toString());
            weiDuVector.append(sqlQuery.value(5).toString());
            ipAddrVector.append(sqlQuery.value(6).toString());
            startTimeVector.append(sqlQuery.value(7).toDateTime().toString("yyyy-MM-dd hh:mm"));
        }
    }
    else
    {
         qDebug() << QObject::tr("查询数据库失败:") << m_database.lastError();
         lockCodeVector.append("");
         lockNameVector.append("");
         userGroupVector.append("");
         lockAddrVector.append("");
         jingDuVector.append("");
         weiDuVector.append("");
         ipAddrVector.append("");
         startTimeVector.append("");
    }
}

void MysqlOperation::getGroupInfo(QVector<QString>& groupVector)
{
    QSqlQuery sqlQuery;
    QString queryStr = "select groupName from groupTable";
    qDebug() << "queryStr = "<<queryStr;
    if(sqlQuery.exec(queryStr))
    {
        while(sqlQuery.next())
        {
            groupVector.append(sqlQuery.value(0).toString());
        }
    }
    else
    {
         qDebug() << QObject::tr("查询数据库失败:") << m_database.lastError();
         groupVector.append("");
    }
}

bool MysqlOperation::createEmpowerTable()
{
    //  0编号 1姓名 2登录名 3所属权限组 4所属用户组 5联系方式 6权限状态
    //  7权限起始日期 8权限截止日期 9下载时间
    QString createTableStr = "create table if not exists empowerTable"
                             "(id integer auto_increment, "
                             "callName varchar(20), "
                             "logName varchar(20), "
                             "powerGroup varchar(20), "
                             "userGroup varchar(50), "
                             "phoneNum varchar(26), "
                             "empower varchar(20), "
                             "empowerST varchar(20), "
                             "empowerET varchar(20), "
                             "loadTime timestamp default current_timestamp, "
                             "primary key(id))";

    QSqlQuery sqlQuery;
    bool ok = sqlQuery.exec(createTableStr);
    if(ok)
    {
        qDebug() << QObject::tr("创建数据库表empowerTable成功");

        return true;
    }
    else
    {
        qDebug() << QObject::tr("创建数据库表empowerTable失败:") << m_database.lastError();
        return false;
    }
}

bool MysqlOperation::insertEmpowerTable(QVector<QString> empowerInfoVector)
{
    QSqlQuery sqlQuery;
    QString format("insert into empowerTable(callName,logName,powerGroup,userGroup,"
                   "phoneNum,empower,empowerST,empowerET) "
                   "values('%1','%2','%3','%4','%5','%6','%7','%8')");
    QString queryStr = format.arg(empowerInfoVector.at(0))
            .arg(empowerInfoVector.at(1))
            .arg(empowerInfoVector.at(2))
            .arg(empowerInfoVector.at(3))
            .arg(empowerInfoVector.at(4))
            .arg(empowerInfoVector.at(5))
            .arg(empowerInfoVector.at(6))
            .arg(empowerInfoVector.at(7));
    qDebug() << "queryStr = "<<queryStr;
    if(sqlQuery.exec(queryStr))
    {
        qDebug() << QObject::tr("插入empower成功");
        return true;
    }
    else
    {
        qDebug() << QObject::tr("插入empower失败");
        return false;
    }
}

int MysqlOperation::getOpenLockCount(QString devCode)
{
    QSqlQuery sqlQuery;
    QString format("select * from recordTable where devCode='%1' and "
                       "(recordType like '0%' or recordType like '%2%')");
    QString queryStr = format.arg(devCode);
    qDebug() << "queryStr = " << queryStr;
    if(sqlQuery.exec(queryStr))
    {
        int count = 0;
        while(sqlQuery.next())
        {
            count++;
        }

        return count;
    }
    else
    {
         qDebug() << QObject::tr("查询数据库失败:") << m_database.lastError();
    }

    return -1;
}

void MysqlOperation::getOpenLockInfo(QString devCode,QVector<QString>&callNameVector,
                                      QVector<QString>&devCodeVector,
                                      QVector<QString>&eventVector,
                                      QVector<QString>&timeVector)
{
    QSqlQuery sqlQuery;
    QString format("select callName,devCode,eventCtx,happenT from recordTable where devCode='%1' and "
                       "(recordType like '0%' or recordType like '%2%') order by id desc");
    QString queryStr = format.arg(devCode);
    qDebug() << "queryStr = " << queryStr;
    if(sqlQuery.exec(queryStr))
    {
        bool flag = false;
        while(sqlQuery.next())
        {
            flag = true;
            callNameVector.append(sqlQuery.value(0).toString());
            devCodeVector.append(sqlQuery.value(1).toString());
            eventVector.append(sqlQuery.value(2).toString());
            timeVector.append(sqlQuery.value(3).toDateTime().toString("yyyy-MM-dd hh:mm"));
        }

        if(!flag)
        {
            callNameVector.append("");
            devCodeVector.append("");
            eventVector.append("");
            timeVector.append("");
        }
    }
    else
    {
         qDebug() << QObject::tr("查询数据库失败:") << m_database.lastError();
         callNameVector.append("");
         devCodeVector.append("");
         eventVector.append("");
         timeVector.append("");
    }
}
