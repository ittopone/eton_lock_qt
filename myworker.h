#ifndef MYWORKER_H
#define MYWORKER_H

#include <QObject>
#include "mysqloperation.h"
#include <QDebug>
#include <QThread>

class MyWorker : public QObject
{
    Q_OBJECT

signals:
    void resultReady(const QString &result);

public slots:
    void doWork(const QString &parameter)
    {
        QString result = tr("in MyWorker doWork:%1").arg(parameter);
        qDebug() << result;
        /* ... here is the expensive or blocking operation ... */
#if 0
        while(1)//该方法不适合死循环
        {
            MysqlOperation::updateEmpower();
            qDebug() << tr("开始进入睡眠60s");
            QThread::sleep(6);
            qDebug() << tr("睡眠60结束");
        }
#else
        MysqlOperation::updateEmpower();
        qDebug() << tr("开始进入睡眠60s");
        QThread::sleep(6);
        qDebug() << tr("睡眠60结束");
#endif
        emit resultReady(result);
    }
};

#endif // WORKER_H
