#ifndef MYCONTROLLER_H
#define MYCONTROLLER_H

#include <QObject>
#include "myworker.h"

class MyController : public QObject
{
    Q_OBJECT
    QThread workerThread;

public:
    MyController()
    {
        MyWorker *worker = new MyWorker;
        worker->moveToThread(&workerThread);
        connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
        connect(this, &MyController::operate, worker, &MyWorker::doWork);
        connect(worker, &MyWorker::resultReady, this, &MyController::handleResults);
        workerThread.start();
    }

    ~MyController()
    {
        qDebug() << tr("调用线程退出函数");
        workerThread.quit();
        workerThread.wait();
    }

signals:
     void operate(const QString &);

public slots:
     void handleResults(const QString & result)
     {
        qDebug() << tr("int MyController handleResults:").arg(result);
     }
};

#endif // CONTROLLER_H
