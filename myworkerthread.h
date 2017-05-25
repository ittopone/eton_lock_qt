#ifndef MYWORKERTHREAD_H
#define MYWORKERTHREAD_H

#include <QThread>
#include <QDebug>
#include "mysqloperation.h"

class MyWorkerThread : public QThread
{
    Q_OBJECT
    void run() Q_DECL_OVERRIDE
    {
        /* ... here is the expensive or blocking operation ... */
#if 1
        while(1)
        {
            MysqlOperation::updateEmpower();
            qDebug() << tr("开始进入睡眠60s");
            QThread::sleep(60);
            qDebug() << tr("睡眠60结束");
        }
#else
        MysqlOperation::updateEmpower();
        qDebug() << tr("开始进入睡眠60s");
        QThread::sleep(60);
        qDebug() << tr("睡眠60结束");
        exec();
#endif
    }
};

#endif // MYWORKERTHREAD_H
