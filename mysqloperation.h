#ifndef MYSQLOPERATION_H
#define MYSQLOPERATION_H
#include <QtSql/QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QVector>

class MysqlOperation
{
public:
    static QSqlDatabase m_database;
    static bool m_dbIsconnect;
    static QString MysqlAddr;
    static QString MysqlDatabaseName;
    static QString MysqlPort;
    static QString MysqlUserName;
    static QString MysqlPassword;
    MysqlOperation();
    static bool openMysqlDatabase();
    static void closeMysqlDatabase();
    static void getGroupInfo(QVector<QString>&);
    //用户管理
    static bool createAdminTable();
    static int isAdminInDatabase(QString, QString);
    static int isAdminEmpower(QString, QString);
    static int isAdminRoot(QString, QString);
    static int isAdminRootBylogName(QString);
    static void getCallNameBylogName(QString,QString&,QString&);
    static void getStEtBylogName(QString,QString&,QString&);
    static bool getAdminInfoByLogName(QString,QVector<QString>&);
    static bool updateAdminInfoByLogName(QString,QVector<QString>);
    static void updateEmpower();
    //锁管理
    static bool createLockTable();
    static QString getLockCodeByIPv4(QString);
    static QString getLockIpBylockCode(QString);
    static QString getLockCodeByLnglat(QString,QString);
    static bool isLockIPTheSame(QString,QString);
    static bool updateLockIP(QString,QString);
    static void getLockNameAddrGroupByCode(QString,QString&,QString&,QString&);
    static void getLockAddrGroupByCode(QString,QString&,QString&);
    static void getLockInfo(QVector<QString>&, QVector<QString>&, QVector<QString>&,
                            QVector<QString>&,QVector<QString>&);
    static void getLockInfoForMap(QVector<QString>&, QVector<QString>&, QVector<QString>&,
                                  QVector<QString>&,QVector<QString>&,QVector<QString>&,
                                  QVector<QString>&,QVector<QString>&);
    //钥匙管理
    static bool createKeyTable();
    static int iskeyIpInDatabase(QString);
    static bool insertIntoKeyTable(QVector<QString>);
    static bool updateSKA(QString,QString);
    static bool updateSKB(QString,QString);
    static bool updateKeyIP(QString,QString);
    static bool updateEmpower(QString,QString,QString,QString,QString);
    static QString getSkaBykeyIP(QString);
    static QString getkeyCodeBykeyIP(QString);
    //日志管理
    static bool createRecordTable();
    static void insertRecord(QString,QString,QString,QString);

    static bool createGroupTable();

    static bool createEmpowerTable();
    static bool insertEmpowerTable(QVector<QString>);
    static int getOpenLockCount(QString);
    static void getOpenLockInfo(QString,QVector<QString>&,QVector<QString>&,
                                 QVector<QString>&,QVector<QString>&);
};

#endif // MYSQLOPERATION_H
