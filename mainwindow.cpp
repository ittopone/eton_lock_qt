#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "selfdefine.h"
#include <QDir>
#include <QDebug>
#include <QVector>
#include "qcomboboxdelegate.h"
#include <QMessageBox>
#include <QSqlError>
#include <QDateTime>
#include "mysqloperation.h"
#include "spcomm.h"

#include <QTextCursor>
#include <QNetworkInterface>
#include <QTreeWidget>
#include <QWebFrame>

//Json头文件
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValue>

#include "exportexcelobject.h"

#include <QFileDialog>
#include <QTimer>
#include <QHostInfo>

//托盘相关
#include <QMenu>

#define WRITE_TIMEOUT 1000
#define READ_TIMEOUT 1000

#define TIMER_TIMEOUT 60000
#define TIMER2_TIMEOUT 5000

MainWindow::MainWindow(QWidget *parent,QString logName) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //关乎界面
    myHelper::FormInCenter(this);
    this->InitStyle();

    //初始化成员变量
    this->m_logName = logName;
    this->m_startServer = false;
    this->m_currentConetCount = 0;
    this->m_selectFilterNum = 10;

    connect(ui->listWidget, SIGNAL(currentRowChanged(int)), this, SLOT(setCurrentStackPageSlot(int)));
    ui->listWidget->setCurrentRow(1);

    //设置QT可以调用javascript函数
    ui->webView->page()->setForwardUnsupportedContent(true);
    ui->webView->page()->settings()->setAttribute(QWebSettings::JavascriptEnabled, true);
    ui->webView->page()->settings()->setAttribute(QWebSettings::JavaEnabled, true);
    ui->webView->settings()->setAttribute(QWebSettings::AutoLoadImages, true);

    connect(ui->webView->page()->mainFrame(),SIGNAL(javaScriptWindowObjectCleared()),this,SLOT(addObjectToJs()));
    connect(ui->webView, SIGNAL(loadFinished(bool)), this, SLOT(myslot_markLockOnBaiduMap(bool)));

    this->updateAdminInfo();

    this->updateAdminTableView();

    this->updateGroupTableView();

    this->updateLockTableView();

    this->updateKeyTableView();

    this->updateEmpowerTableView();

    this->updateGroupComboBox();

    this->updateLoadEmpowerTableView();

    this->updateLockTableWidget();

    //初始化状态栏
    this->serIPLabel = new QLabel;

    this->serPortLabel = new QLabel;

    this->serStatusLabel = new QLabel;

    this->currentNumLabel = new QLabel;

    this->maxNumLabel = new QLabel;

    this->m_progressBar = new QProgressBar;

    this->initStatusBar();

    MainWindow::connect(ui->webView, SIGNAL(loadProgress(int)), m_progressBar, SLOT(setValue(int)));

    //服务器
    ui->lineEdit_29->setText(this->getLocalHostIP().toString());

    connect(ui->spinBox,SIGNAL(valueChanged(int)),this,SLOT(myslot_updateStatusBarMaxCount(int)));

    connect(ui->spinBox_2,SIGNAL(valueChanged(int)),this,SLOT(myslot_updateStatusBarPort(int)));

    //更新当前可用串口号
    this->updatePortNameComboBox();

    connect(&m_adminModel,SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),
            ui->tableView_2,SLOT(resizeColumnsToContents()));

    connect(&m_keyModel,SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),
            ui->tableView_7,SLOT(resizeColumnsToContents()));

    connect(&m_lockModel,SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),
            ui->tableView_5,SLOT(resizeColumnsToContents()));

    connect(ui->textBrowser,SIGNAL(cursorPositionChanged()),this,SLOT(autoScroll()));


    //读取服务器配置信息
    QFile fileS("ServerConfig.ini");
    if (fileS.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&fileS);
        for(int lineNum = 0; !in.atEnd(); lineNum++)
        {
            QString line = in.readLine();
            qDebug() << line;
            switch(lineNum)
            {
            case 0:
                ui->spinBox->setValue(line.toInt(NULL, 10));
                break;
            case 1:
                ui->spinBox_2->setValue(line.toInt(NULL, 10));
                break;
            case 2:
                if(line == "1")
                {
                    ui->checkBox->setChecked(true);
                    //启动服务器
                    if(!this->startServer())
                    {
#ifdef USE_SELF_MSGBOX
                        myHelper::ShowMessageBoxError(QObject::tr("启动服务器失败"));
#else
                        QMessageBox::warning(NULL, QObject::tr("警告"),
                        QObject::tr("启动服务器失败"), QMessageBox::Yes);
#endif
                    }
                }
            default:
                break;
            }
        }

        fileS.close();
    }

    this->updateRecordTableView(10);

    //与托盘相关
    QSystemTrayIcon *trayIcon;

    QAction *minimizeAction;

    QAction *restoreAction;

    QAction *quitAction;

    QMenu   *trayIconMenu;

    //创建托盘图标
    QIcon icon = QIcon(":/image/eton.ico");
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(icon);
    trayIcon->setToolTip(tr("易网通监控平台"));
    QString titlec=tr("易网通监控平台");
    QString textc=tr("易网通监控平台：嗨，欢迎使用");
    trayIcon->show();

    //弹出气泡提示
    trayIcon->showMessage(titlec,textc,QSystemTrayIcon::Information,2000);

    //添加单/双击鼠标相应
    connect(trayIcon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this,SLOT(myslot_trayIconActivated(QSystemTrayIcon::ActivationReason)));

    //创建监听行为
    minimizeAction = new QAction(tr("最小化 (&I)"), this);
    connect(minimizeAction, SIGNAL(triggered()), this, SLOT(hide()));
    restoreAction = new QAction(tr("还原 (&R)"), this);
    connect(restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));
    quitAction = new QAction(tr("退出 (&Q)"), this);
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));
//    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));

    //创建右键弹出菜单
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(minimizeAction);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);
    trayIcon->setContextMenu(trayIconMenu);
#if 0
    //创建定时器1
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(myslot_timeOutExec()));
    timer->start(TIMER_TIMEOUT);
#endif
    //创建定时器2
    QTimer *time2 = new QTimer(this);
    connect(time2, SIGNAL(timeout()), this, SLOT(myslot_checkLockOpenStatusTimeOut()));
    time2->start(TIMER2_TIMEOUT);

    if(MysqlOperation::isAdminRootBylogName(m_logName) == 1)
    {
        ui->pushButton_30->setEnabled(true);
        ui->pushButton_31->setEnabled(true);
        ui->pushButton_32->setEnabled(true);
        ui->pushButton_27->setEnabled(true);

        ui->lineEdit->setEnabled(true);
        ui->comboBox->setEnabled(true);
        ui->comboBox_2->setEnabled(true);
        ui->comboBox_13->setEnabled(true);
        ui->dateTimeEdit->setEnabled(true);
        ui->dateTimeEdit_2->setEnabled(true);
    }
    else
    {
        int count = ui->listWidget->count();
        for(int index=0; index < count; index++)
        {
            if(ui->listWidget->item(index)->text().trimmed() == QObject::tr("授权管理"))
            {
                ui->listWidget->takeItem(index);
                count = ui->listWidget->count();
                index=0;
                continue;
            }
            if(ui->listWidget->item(index)->text().trimmed() == QObject::tr("用户管理"))
            {
                ui->listWidget->takeItem(index);
                count = ui->listWidget->count();
                index=0;
                continue;
            }
        }
    }

    //开启线程
#if 0
    m_controller = new MyController;
    m_controller->operate(tr("text by controller"));
#else
    m_workerThread = new MyWorkerThread;
    m_workerThread->start();
#endif
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addObjectToJs()
{
    ui->webView->page()->mainFrame()->addToJavaScriptWindowObject("MainWindow",this);
}

QString MainWindow::jsInvokQt(QString jingDu,QString weiDu)
{
    qDebug() << tr("经纬度:") << jingDu << weiDu;
    QString lockCode = MysqlOperation::getLockCodeByLnglat(jingDu,weiDu);
    if(!(lockCode.isNull())||!(lockCode.isEmpty()))
    {
        QVector<QString> callNameVector;
        QVector<QString> devCodeVector;
        QVector<QString> eventVector;
        QVector<QString> timeVector;
        MysqlOperation::getOpenLockInfo(lockCode,callNameVector,devCodeVector,eventVector,timeVector);
        QString format("<tr bgcolor='#5CACEE'>"
                       "<td>%1</td>"
                       "<td>%2</td>"
                       "<td>%3</td>"
                       "<td>%4</td>"
                       "</tr>");
        QString content("<table border='1'>"
                        "<tr bgcolor='#1B89CA'>"
                        "<th>姓名</th>"
                        "<th>锁编号</th>"
                        "<th>内容</th>"
                        "<th>开锁时间</th>"
                        "</tr>");
        int count = callNameVector.size();
        if(count > 10)
        {
            count = 10;
        }
        for(int i=0; i < count; i++)
        {
            content += format.arg(callNameVector.at(i)).arg(devCodeVector.at(i))
                    .arg(eventVector.at(i)).arg(timeVector.at(i));
        }

        content.append("</table>");
        return content;
    }
    return NULL;
}

void MainWindow::myslot_checkLockOpenStatusTimeOut()
{
    if(!m_startServer)
    {
        qDebug() << tr("服务器未启动");
        return;
    }

    QMapIterator<QString, QTcpSocket*> iterator(m_mapDevCodeIsKey);
    while(iterator.hasNext())
    {
        iterator.next();
        QString devCode = iterator.key();
        qDebug() << QObject::tr("timer deal:'%1").arg(devCode);

        this->getLockOpenStatus(devCode);
    }
}


int MainWindow::getLockOpenStatus(QString devCode)
{
    char sendDevCmd[8];
    sendDevCmd[0] = 0x01;
    sendDevCmd[1] = 0x03;
    sendDevCmd[2] = 0x01;
    sendDevCmd[3] = 0x00;
    sendDevCmd[4] = 0x00;
    sendDevCmd[5] = 0x01;

    int crc = -1;
    crc = this->getCRC16(sendDevCmd,6,NULL);
    if(crc == -1)
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("获取校验码失败"));
#else
        QMessageBox::warning(NULL, QObject::tr("警告"),
                             QObject::tr("获取校验码失败"),
                             QMessageBox::Yes);
#endif
        return -1;
    }

    sendDevCmd[6] = crc & 0xff;
    sendDevCmd[7] = (crc >> 8) & 0xff;

    if(!m_mapDevCodeIsKey.contains(devCode))
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("编码为%1的设备未连接").arg(devCode));
#else
        QMessageBox::warning(NULL, QObject::tr("警告"),
                             QObject::tr("编码为%1的设备未连接").arg(devCode),
                             QMessageBox::Yes);
#endif
        return -1;
    }

    QTcpSocket *devSocket = m_mapDevCodeIsKey.value(devCode);

    if(devSocket->write(sendDevCmd, 8) != -1)
    {
        if(!devSocket->waitForBytesWritten(WRITE_TIMEOUT))
        {
#ifdef USE_SELF_MSGBOX
            myHelper::ShowMessageBoxError(QObject::tr("网络超时").arg(devSocket->errorString()));
#else
            QMessageBox::warning(NULL, QObject::tr("警告"),
                                 QObject::tr("网络超时")
                                 .arg(devSocket->errorString()),
                                 QMessageBox::Yes);
#endif
            return -1;
        }

        if(!devSocket->waitForReadyRead(READ_TIMEOUT))
        {
#ifdef USE_SELF_MSGBOX
            myHelper::ShowMessageBoxError(QObject::tr("网络超时").arg(devSocket->errorString()));
#else
            QMessageBox::warning(NULL, QObject::tr("警告"),
                                 QObject::tr("网络超时")
                                 .arg(devSocket->errorString()),
                                 QMessageBox::Yes);
#endif
            return -1;
        }

        QByteArray readbuf = devSocket->readAll();
        if(readbuf.isEmpty()||readbuf.isNull())
        {
#ifdef USE_SELF_MSGBOX
            myHelper::ShowMessageBoxError(QObject::tr("网络超时").arg(devSocket->errorString()));
#else
            QMessageBox::warning(NULL, QObject::tr("警告"),
                                 QObject::tr("网络错误")
                                 .arg(devSocket->errorString()),
                                 QMessageBox::Yes);
#endif
            return -1;
        }

        const char *pchar = readbuf.constData();
        QString readStr;
        for(int n = 0; n < readbuf.length(); n++)
        {
            readStr += QString().sprintf("%02X", (unsigned char)*(pchar + n));
        }
        qDebug() << QObject::tr("从设备中读取到DI1返回:%1").arg(readStr);
        if(readStr.contains("0103020000"))
        {
             for(int row=0; row < ui->tableWidget_2->rowCount(); row++)
             {
                 if(ui->tableWidget_2->item(row, 1)->text() == devCode)
                 {
                     ui->tableWidget_2->item(row, 6)->setText(QObject::tr("开"));
                     break;
                 }
             }

            return 1;
        }
        else
        {
            for(int row=0; row < ui->tableWidget_2->rowCount(); row++)
            {
                if(ui->tableWidget_2->item(row, 1)->text() == devCode)
                {
                    ui->tableWidget_2->item(row, 6)->setText(QObject::tr("关"));
                    break;
                }
            }

           return 0;
        }
    }
    else
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("网络超时").arg(devSocket->errorString()));
#else
        QMessageBox::warning(NULL, QObject::tr("警告"),
                             QObject::tr("网络错误")
                             .arg(devSocket->errorString()),
                             QMessageBox::Yes);
#endif
        return -1;
    }
}

int MainWindow::setLockModeAuto(QString devCode)
{
    char sendDevCmd[8];
    sendDevCmd[0] = 0x01;
    sendDevCmd[1] = 0x06;
    sendDevCmd[2] = 0x02;
    sendDevCmd[3] = 0x00;
    sendDevCmd[4] = 0x00;
    sendDevCmd[5] = 0x00;

    int crc = -1;
    crc = this->getCRC16(sendDevCmd,6,NULL);
    if(crc == -1)
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("获取校验码失败"));
#else
        QMessageBox::warning(NULL, QObject::tr("警告"),
                             QObject::tr("获取校验码失败"),
                             QMessageBox::Yes);
#endif
        return -1;
    }

    sendDevCmd[6] = crc & 0xff;
    sendDevCmd[7] = (crc >> 8) & 0xff;

    if(!m_mapDevCodeIsKey.contains(devCode))
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("编码为%1的设备未连接").arg(devCode));
#else
        QMessageBox::warning(NULL, QObject::tr("警告"),
                             QObject::tr("编码为%1的设备未连接").arg(devCode),
                             QMessageBox::Yes);
#endif
        return -1;
    }

    QTcpSocket *devSocket = m_mapDevCodeIsKey.value(devCode);

    if(devSocket->write(sendDevCmd, 8) != -1)
    {
        if(!devSocket->waitForBytesWritten(WRITE_TIMEOUT))
        {
#ifdef USE_SELF_MSGBOX
            myHelper::ShowMessageBoxError(QObject::tr("网络超时").arg(devSocket->errorString()));
#else
            QMessageBox::warning(NULL, QObject::tr("警告"),
                                 QObject::tr("网络超时")
                                 .arg(devSocket->errorString()),
                                 QMessageBox::Yes);
#endif
            return -1;
        }

        if(!devSocket->waitForReadyRead(READ_TIMEOUT))
        {
#ifdef USE_SELF_MSGBOX
            myHelper::ShowMessageBoxError(QObject::tr("网络超时").arg(devSocket->errorString()));
#else
            QMessageBox::warning(NULL, QObject::tr("警告"),
                                 QObject::tr("网络超时")
                                 .arg(devSocket->errorString()),
                                 QMessageBox::Yes);
#endif
            return -1;
        }

        QByteArray readbuf = devSocket->readAll();
        if(readbuf.isEmpty()||readbuf.isNull())
        {
#ifdef USE_SELF_MSGBOX
            myHelper::ShowMessageBoxError(QObject::tr("网络超时").arg(devSocket->errorString()));
#else
            QMessageBox::warning(NULL, QObject::tr("警告"),
                                 QObject::tr("网络错误")
                                 .arg(devSocket->errorString()),
                                 QMessageBox::Yes);
#endif
            return -1;
        }

        const char *pchar = readbuf.constData();
        QString readStr;
        for(int n = 0; n < readbuf.length(); n++)
        {
            readStr += QString().sprintf("%02X", (unsigned char)*(pchar + n));
        }
        qDebug() << QObject::tr("从设备中读取到:%1").arg(readStr);
        if(readStr.contains("010602FFFF"))
        {
#ifdef USE_SELF_MSGBOX
            myHelper::ShowMessageBoxError(QObject::tr("设备返回错误信息"));
#else
            QMessageBox::warning(NULL, QObject::tr("警告"),
                                 QObject::tr("设备返回错误信息"),
                                 QMessageBox::Yes);
#endif
            return -1;
        }
    }
    else
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("网络超时").arg(devSocket->errorString()));
#else
        QMessageBox::warning(NULL, QObject::tr("警告"),
                             QObject::tr("网络错误")
                             .arg(devSocket->errorString()),
                             QMessageBox::Yes);
#endif
        return -1;
    }

    return 0;
}

int MainWindow::openLock(QString devCode)
{
    char sendDevCmd[8];
    sendDevCmd[0] = 0x01;
    sendDevCmd[1] = 0x06;
    sendDevCmd[2] = 0x02;
    sendDevCmd[3] = 0x0A;
    sendDevCmd[4] = 0x00;
    sendDevCmd[5] = 0x01;

    int crc = -1;
    crc = this->getCRC16(sendDevCmd,6,NULL);
    if(crc == -1)
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("获取校验码失败"));
#else
        QMessageBox::warning(NULL, QObject::tr("警告"),
                             QObject::tr("获取校验码失败"),
                             QMessageBox::Yes);
#endif
        return -1;
    }

    sendDevCmd[6] = crc & 0xff;
    sendDevCmd[7] = (crc >> 8) & 0xff;

    if(!m_mapDevCodeIsKey.contains(devCode))
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("编码为%1的设备未连接").arg(devCode));
#else
        QMessageBox::warning(NULL, QObject::tr("警告"),
                             QObject::tr("编码为%1的设备未连接").arg(devCode),
                             QMessageBox::Yes);
#endif
        return -1;
    }

    QTcpSocket *devSocket = m_mapDevCodeIsKey.value(devCode);

    if(devSocket->write(sendDevCmd, 8) != -1)
    {
        if(!devSocket->waitForBytesWritten(WRITE_TIMEOUT))
        {
#ifdef USE_SELF_MSGBOX
            myHelper::ShowMessageBoxError(QObject::tr("网络超时").arg(devSocket->errorString()));
#else
            QMessageBox::warning(NULL, QObject::tr("警告"),
                                 QObject::tr("网络超时")
                                 .arg(devSocket->errorString()),
                                 QMessageBox::Yes);
#endif
            return -1;
        }

        if(!devSocket->waitForReadyRead(READ_TIMEOUT))
        {
#ifdef USE_SELF_MSGBOX
            myHelper::ShowMessageBoxError(QObject::tr("网络超时").arg(devSocket->errorString()));
#else
            QMessageBox::warning(NULL, QObject::tr("警告"),
                                 QObject::tr("网络超时")
                                 .arg(devSocket->errorString()),
                                 QMessageBox::Yes);
#endif
            return -1;
        }

        QByteArray readbuf = devSocket->readAll();
        if(readbuf.isEmpty()||readbuf.isNull())
        {
#ifdef USE_SELF_MSGBOX
            myHelper::ShowMessageBoxError(QObject::tr("网络超时").arg(devSocket->errorString()));
#else
            QMessageBox::warning(NULL, QObject::tr("警告"),
                                 QObject::tr("网络超时")
                                 .arg(devSocket->errorString()),
                                 QMessageBox::Yes);
#endif
            return -1;
        }

        const char *pchar = readbuf.constData();
        QString readStr;
        for(int n = 0; n < readbuf.length(); n++)
        {
            readStr += QString().sprintf("%02X", (unsigned char)*(pchar + n));
        }
        qDebug() << QObject::tr("从设备中读取到:%1").arg(readStr);
        if(readStr.contains("010602FFFF"))
        {
#ifdef USE_SELF_MSGBOX
            myHelper::ShowMessageBoxError(QObject::tr("设备返回错误信息"));
#else
            QMessageBox::warning(NULL, QObject::tr("警告"),
                                 QObject::tr("设备返回错误信息"),
                                 QMessageBox::Yes);
#endif
            return -1;
        }
    }
    else
    {
#ifdef USE_SELF_MSGBOX
            myHelper::ShowMessageBoxError(QObject::tr("网络超时").arg(devSocket->errorString()));
#else
            QMessageBox::warning(NULL, QObject::tr("警告"),
                                 QObject::tr("网络错误")
                                 .arg(devSocket->errorString()),
                                 QMessageBox::Yes);
#endif
        return -1;
    }

    return 0;
}

int MainWindow::setLockModeHand(QString devCode)
{
    char sendDevCmd[8];
    sendDevCmd[0] = 0x01;
    sendDevCmd[1] = 0x06;
    sendDevCmd[2] = 0x02;
    sendDevCmd[3] = 0x00;
    sendDevCmd[4] = 0x00;
    sendDevCmd[5] = 0x0B;

    int crc = -1;
    crc = this->getCRC16(sendDevCmd,6,NULL);
    if(crc == -1)
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("获取校验码失败"));
#else
        QMessageBox::warning(NULL, QObject::tr("警告"),
                             QObject::tr("获取校验码失败"),
                             QMessageBox::Yes);
#endif
        return -1;
    }

    sendDevCmd[6] = crc & 0xff;
    sendDevCmd[7] = (crc >> 8) & 0xff;

    if(!m_mapDevCodeIsKey.contains(devCode))
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("编码为%1的设备未连接").arg(devCode));
#else
        QMessageBox::warning(NULL, QObject::tr("警告"),
                             QObject::tr("编码为%1的设备未连接").arg(devCode),
                             QMessageBox::Yes);
#endif
        return -1;
    }

    QTcpSocket *devSocket = m_mapDevCodeIsKey.value(devCode);

    if(devSocket->write(sendDevCmd, 8) != -1)
    {
        if(!devSocket->waitForBytesWritten(WRITE_TIMEOUT))
        {
#ifdef USE_SELF_MSGBOX
            myHelper::ShowMessageBoxError(QObject::tr("网络超时").arg(devSocket->errorString()));
#else
            QMessageBox::warning(NULL, QObject::tr("警告"),
                                 QObject::tr("网络超时")
                                 .arg(devSocket->errorString()),
                                 QMessageBox::Yes);
#endif
            return -1;
        }

        if(!devSocket->waitForReadyRead(READ_TIMEOUT))
        {
#ifdef USE_SELF_MSGBOX
            myHelper::ShowMessageBoxError(QObject::tr("网络超时").arg(devSocket->errorString()));
#else
            QMessageBox::warning(NULL, QObject::tr("警告"),
                                 QObject::tr("网络超时")
                                 .arg(devSocket->errorString()),
                                 QMessageBox::Yes);
#endif
            return -1;
        }

        QByteArray readbuf = devSocket->readAll();
        if(readbuf.isEmpty()||readbuf.isNull())
        {
#ifdef USE_SELF_MSGBOX
            myHelper::ShowMessageBoxError(QObject::tr("网络超时").arg(devSocket->errorString()));
#else
            QMessageBox::warning(NULL, QObject::tr("警告"),
                                 QObject::tr("网络错误")
                                 .arg(devSocket->errorString()),
                                 QMessageBox::Yes);
#endif
            return -1;
        }

        char *pchar = readbuf.data();
        QString readStr;
        for(int n = 0; n < readbuf.length(); n++)
        {
            readStr += QString().sprintf("%02X", (unsigned char)*(pchar + n));
        }
        qDebug() << QObject::tr("从设备中读取到:%1").arg(readStr);
        if(readStr.contains("010602FFFF"))
        {
#ifdef USE_SELF_MSGBOX
            myHelper::ShowMessageBoxError(QObject::tr("设备返回错误信息"));
#else
            QMessageBox::warning(NULL, QObject::tr("警告"),
                                 QObject::tr("设备返回错误信息"),
                                 QMessageBox::Yes);
#endif
            return -1;
        }
    }
    else
    {
#ifdef USE_SELF_MSGBOX
            myHelper::ShowMessageBoxError(QObject::tr("网络超时").arg(devSocket->errorString()));
#else
            QMessageBox::warning(NULL, QObject::tr("警告"),
                                 QObject::tr("网络错误")
                                 .arg(devSocket->errorString()),
                                 QMessageBox::Yes);
#endif
        return -1;
    }

    return 0;
}

void MainWindow::sleep(unsigned int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while( QTime::currentTime() < dieTime )
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

}

int MainWindow::getCRC16(char *bufData, int buflen, char *pcrc)
{
    unsigned short CRC = 0xffff;
    unsigned short POLYNOMIAL = 0xa001;
    if (bufData == NULL)
    {
        return -1;
    }
    if (buflen == 0)
    {
        return -1;
    }
    for (int i = 0; i < buflen; i++)
    {
        CRC ^= bufData[i];
        for (int j = 0; j < 8; j++)
        {
            if ((CRC & 0x0001) != 0)
            {
                CRC >>= 1;
                CRC ^= POLYNOMIAL;
            }
            else
            {
                CRC >>= 1;
            }
        }
    }

    if(pcrc)
    {
        pcrc[0] = CRC & 0x00ff;
        pcrc[1] = CRC >> 8;
    }

    qDebug() << QString::number(CRC, 16);

    return CRC;
}

void MainWindow::InitStyle()
{
    //设置窗体标题栏隐藏
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint);
    location = this->geometry();
    max = false;
    mousePressed = false;

    //安装事件监听器,让标题栏识别鼠标双击
    ui->lab_Title->installEventFilter(this);

    IconHelper::Instance()->SetIcon(ui->btnMenu_Close, QChar(0xf00d), 10);
    IconHelper::Instance()->SetIcon(ui->btnMenu_Max, QChar(0xf096), 10);
    IconHelper::Instance()->SetIcon(ui->btnMenu_Min, QChar(0xf068), 10);
    IconHelper::Instance()->SetIcon(ui->btnMenu, QChar(0xf0c9), 10);
    IconHelper::Instance()->SetIcon(ui->lab_Ico, QChar(0xf015), 12);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonDblClick) {
        this->on_btnMenu_Max_clicked();
        return true;
    }
    return QObject::eventFilter(obj, event);
}

void MainWindow::mouseMoveEvent(QMouseEvent *e)
{
    if (mousePressed && (e->buttons() && Qt::LeftButton) && !max) {
        this->move(e->globalPos() - mousePoint);
        e->accept();
    }
}

void MainWindow::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        mousePressed = true;
        mousePoint = e->globalPos() - this->pos();
        e->accept();
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *)
{
    mousePressed = false;
}

void MainWindow::on_btnMenu_Close_clicked()
{
    qApp->closeAllWindows();
    qApp->exit();
}

void MainWindow::on_btnMenu_Max_clicked()
{
    if (max) {
        this->setGeometry(location);
        IconHelper::Instance()->SetIcon(ui->btnMenu_Max, QChar(0xf096), 10);
        ui->btnMenu_Max->setToolTip("最大化");
    } else {
        location = this->geometry();
        this->setGeometry(qApp->desktop()->availableGeometry());
        IconHelper::Instance()->SetIcon(ui->btnMenu_Max, QChar(0xf079), 10);
        ui->btnMenu_Max->setToolTip("还原");
    }
    max = !max;
}

void MainWindow::on_btnMenu_Min_clicked()
{
    this->showMinimized();
}
#if 0
void MainWindow::myslot_timeOutExec()
{
    MysqlOperation::updateEmpower();
}
#endif
void MainWindow::updateRecordTableView(int selectFilterNum)
{
    ui->dateTimeEdit_4->setDateTime(QDateTime::currentDateTime());

    m_recordModel.setTable("recordTable");
    m_recordModel.setEditStrategy(QSqlTableModel::OnManualSubmit);

    //设置过滤器
    switch(selectFilterNum)
    {
    case 0:
        m_recordModel.setFilter("recordType like '%0%'");
        m_selectFilterNum = 0;
        break;
    case 1:
        m_recordModel.setFilter("recordType like '%1%'");
        m_selectFilterNum = 1;
        break;
    case 2:
        m_recordModel.setFilter("recordType like '%2%'");
        m_selectFilterNum = 2;
        break;
    case 3:
        m_recordModel.setFilter("recordType like '%3%'");
        m_selectFilterNum = 3;
        break;
    case 4:
        m_recordModel.setFilter("recordType like '%4%'");
        m_selectFilterNum = 4;
        break;
    case 5:
        m_recordModel.setFilter("recordType like '%5%'");
        m_selectFilterNum = 5;
        break;
    case 6:
        m_recordModel.setFilter("recordType like '%6%'");
        m_selectFilterNum = 6;
        break;
    case 7:
        m_recordModel.setFilter("recordType like '%7%'");
        m_selectFilterNum = 7;
        break;
    default:
        m_selectFilterNum = 10;
        break;
    }

    m_recordModel.select();
    m_recordModel.setHeaderData(1, Qt::Horizontal, QObject::tr("姓名"));
    m_recordModel.setHeaderData(2, Qt::Horizontal, QObject::tr("设备编号"));
    m_recordModel.setHeaderData(4, Qt::Horizontal, QObject::tr("事件内容"));
    m_recordModel.setHeaderData(5, Qt::Horizontal, QObject::tr("发生时间"));

    ui->tableView_9->setModel(&m_recordModel);
    ui->tableView_9->hideColumn(0);
    ui->tableView_9->hideColumn(3);

    ui->tableView_9->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

void MainWindow::myslot_markLockOnBaiduMap(bool status)
{
    if(!status)
    {
        return;
    }

    qDebug() << tr("网页已经加载完毕");

    QVector<QString> lockCodeVector;
    QVector<QString> lockNameVector;
    QVector<QString> userGroupVector;
    QVector<QString> lockAddrVector;
    QVector<QString> jingDuVector;
    QVector<QString> weiDuVector;
    QVector<QString> ipAddrVector;
    QVector<QString> startTimeVector;

    MysqlOperation::getLockInfoForMap(lockCodeVector,lockNameVector,
                                           userGroupVector,lockAddrVector,
                                           jingDuVector,weiDuVector,
                                           ipAddrVector,startTimeVector);

    //加载本地的锁图标
    QString dirStr = QDir::currentPath();
    QString iconPath("file:///" + dirStr + "/lock.png");
    qDebug() << tr("锁图标加载路径:") << iconPath;
    ui->webView->page()->mainFrame()->evaluateJavaScript(QString("getIconPath(\"%1\")").arg(iconPath));

    int count = lockCodeVector.size();
    for(int i=0; i < count; i++)
    {
        int count = MysqlOperation::getOpenLockCount(lockCodeVector.at(i));
        QString cntentFormat("<table border='1'>"
                             "<tr>"
                             "<th bgcolor='#1B89CA'>编号</th>"
                             "<td bgcolor='#5CACEE'>%1</td>"
                             "</tr>"
                             "<tr>"
                             "<th bgcolor='#1B89CA'>名称</th>"
                             "<td bgcolor='#5CACEE'>%2</td>"
                             "</tr>"
                             "<tr>"
                             "<th bgcolor='#1B89CA'>用户组</th>"
                             "<td bgcolor='#5CACEE'>%3</td>"
                             "</tr>"
                             "<tr>"
                             "<th bgcolor='#1B89CA'>地址</th>"
                             "<td bgcolor='#5CACEE'>%4</td>"
                             "</tr>"
                             "<tr>"
                             "<th bgcolor='#1B89CA'>经纬度</th>"
                             "<td bgcolor='#5CACEE'>%5,%6</td>"
                             "</tr>"
                             "<tr>"
                             "<th bgcolor='#1B89CA'>fsuIP地址</th>"
                             "<td bgcolor='#5CACEE'>%7</td>"
                             "</tr>"
                             "<tr>"
                             "<th bgcolor='#1B89CA'>服役时间</th>"
                             "<td bgcolor='#5CACEE'>%8</td>"
                             "</tr>"
                             "<tr>"
                             "<th bgcolor='#1B89CA'>开锁次数</th>"
                             "<td bgcolor='#5CACEE'>%9</td>"
                             "</tr>");
        QString lockContent = cntentFormat.arg(lockCodeVector.at(i)).arg(lockNameVector.at(i))
                .arg(userGroupVector.at(i)).arg(lockAddrVector.at(i)).arg(jingDuVector.at(i))
                .arg(weiDuVector.at(i)).arg(ipAddrVector.at(i)).arg(startTimeVector.at(i))
                .arg(QString::number(count));

        QString jsFormat("markLockOnBaiduMap(\"%1\",\"%2\",\"%3\")");
        QString jsFun = jsFormat.arg(jingDuVector.at(i)).arg(weiDuVector.at(i)).arg(lockContent);
        qDebug() << tr("调用js语句:") << jsFun;
        ui->webView->page()->mainFrame()->evaluateJavaScript(jsFun);
    }

}

bool MainWindow::socketWriteAndRead(QTcpSocket *Socket,char *pchStr, QByteArray& readBuf)
{
    //发送
    qDebug() << tr("发送:") << pchStr;
    if(Socket->write(pchStr, 128) != -1)
    {
        if(!Socket->waitForBytesWritten(WRITE_TIMEOUT))
        {
#ifdef USE_SELF_MSGBOX
            myHelper::ShowMessageBoxError(QObject::tr("网络错误").arg(Socket->errorString()));
#else
            QMessageBox::warning(NULL, QObject::tr("警告"),
                                 QObject::tr("网络错误")
                                 .arg(Socket->errorString()),
                                 QMessageBox::Yes);
#endif

            return false;
        }

        //接收
        if(!Socket->waitForReadyRead(READ_TIMEOUT))
        {
#ifdef USE_SELF_MSGBOX
            myHelper::ShowMessageBoxError(QObject::tr("网络超时").arg(Socket->errorString()));
#else
            QMessageBox::warning(NULL, QObject::tr("警告"),
                                 QObject::tr("网络超时")
                                 .arg(Socket->errorString()),
                                 QMessageBox::Yes);
#endif

            return false;
        }
        readBuf = Socket->readAll();
        qDebug() << tr("接收:") << readBuf;
        if(readBuf.isEmpty()||readBuf.isNull())
        {
#ifdef USE_SELF_MSGBOX
            myHelper::ShowMessageBoxError(QObject::tr("网络错误").arg(Socket->errorString()));
#else
            QMessageBox::warning(NULL, QObject::tr("警告"),
                                 QObject::tr("网络错误")
                                 .arg(Socket->errorString()),
                                 QMessageBox::Yes);
#endif

            return false;
        }

        return true;
    }
    else
    {
#ifdef USE_SELF_MSGBOX
            myHelper::ShowMessageBoxError(QObject::tr("网络错误").arg(Socket->errorString()));
#else
            QMessageBox::warning(NULL, QObject::tr("警告"),
                                 QObject::tr("网络错误")
                                 .arg(Socket->errorString()),
                                 QMessageBox::Yes);
#endif

        return false;
    }
}

bool MainWindow::getLockAddress(QTcpSocket *lockSocket, QString &lockHexIP)
{
    char *pchStr = new char[128];
    char *chkSum = new char;
    sprintf_s(pchStr, 128, "FFFFFFFFRA,");
    SPComm::getChksum(pchStr, chkSum);
    sprintf_s(pchStr, 128, "#FFFFFFFFRA,%02X\r", *chkSum & 0xff);

    qDebug() << tr("发送锁hex地址指令:") << pchStr;

    //发送
    if(lockSocket->write(pchStr, 128) != -1)
    {
        if(!lockSocket->waitForBytesWritten(WRITE_TIMEOUT))
        {
#ifdef USE_SELF_MSGBOX
            myHelper::ShowMessageBoxError(QObject::tr("网络超时").arg(lockSocket->errorString()));
#else
            QMessageBox::warning(NULL, QObject::tr("警告"),
                                 QObject::tr("网络超时")
                                 .arg(lockSocket->errorString()),
                                 QMessageBox::Yes);
#endif
            delete[] pchStr;
            pchStr = NULL;
            delete chkSum;
            chkSum = NULL;

            return false;
        }

        //接收
        if(!lockSocket->waitForReadyRead(READ_TIMEOUT))
        {
#ifdef USE_SELF_MSGBOX
            myHelper::ShowMessageBoxError(QObject::tr("网络超时").arg(lockSocket->errorString()));
#else
            QMessageBox::warning(NULL, QObject::tr("警告"),
                                 QObject::tr("网络超时")
                                 .arg(lockSocket->errorString()),
                                 QMessageBox::Yes);
#endif
            delete[] pchStr;
            pchStr = NULL;
            delete chkSum;
            chkSum = NULL;

            return false;
        }
        QByteArray readbuf = lockSocket->readAll();
        qDebug() << tr("接收:") << readbuf;
        if(readbuf.isEmpty()||readbuf.isNull())
        {
#ifdef USE_SELF_MSGBOX
            myHelper::ShowMessageBoxError(QObject::tr("网络错误").arg(lockSocket->errorString()));
#else
            QMessageBox::warning(NULL, QObject::tr("警告"),
                                 QObject::tr("网络错误")
                                 .arg(lockSocket->errorString()),
                                 QMessageBox::Yes);
#endif

            delete[] pchStr;
            pchStr = NULL;
            delete chkSum;
            chkSum = NULL;

            return false;
        }

        //判断
        if(!readbuf.contains("??"))
        {
            lockHexIP = readbuf.mid(11, 8);
            qDebug() << "锁hex地址为:" <<lockHexIP;

            delete[] pchStr;
            pchStr = NULL;
            delete chkSum;
            chkSum = NULL;

            return true;
        }
    }
    else
    {
        delete[] pchStr;
        pchStr = NULL;
        delete chkSum;
        chkSum = NULL;

        return false;
    }
}

void MainWindow::myslot_trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch(reason)
    {
    case QSystemTrayIcon::Trigger:
        //单击托盘图标
        this->showNormal();
        this->raise();
        break;
    case QSystemTrayIcon::DoubleClick:
        //双击托盘图标
        this->showNormal();
        this->raise();
        break;
    default:
        break;
    }
}

void MainWindow::closeEvent(QCloseEvent * event)
{

    if(ui->comboBox->currentText() == tr("操作员"))
    {
        MysqlOperation::insertRecord(ui->lineEdit->text().trimmed(),"","NNNNNN6N",QObject::tr("已退出登录"));
        if(m_startServer)
        {
            MysqlOperation::insertRecord(ui->lineEdit->text().trimmed(),"","NNNNN56N",QObject::tr("已关闭服务器"));
        }
    }
    else
    {
        MysqlOperation::insertRecord(ui->lineEdit->text().trimmed(),"","NNNNNNN7",QObject::tr("已退出登录"));
        if(m_startServer)
        {
            MysqlOperation::insertRecord(ui->lineEdit->text().trimmed(),"","NNNNN5N7",QObject::tr("已关闭服务器"));
        }
    }
#if 0
    //程序退出来后线程还是继续执行，以下方法无法结束线程
    delete m_controller;
    m_controller = NULL;
#else
    m_workerThread->exit();
#endif

    MysqlOperation::closeMysqlDatabase();

}

void MainWindow::updateAdminInfo()
{

    QVector<QString> adminInfoVector;
    if(!MysqlOperation::getAdminInfoByLogName(m_logName,adminInfoVector))
    {
        return;
    }

    ui->lineEdit->setText(adminInfoVector.at(0));//姓名
    ui->lineEdit_2->setText(adminInfoVector.at(1));//登录名
    ui->lineEdit_3->setText(adminInfoVector.at(2));//密码

    ui->comboBox->setCurrentText(adminInfoVector.at(3));//权限组
    ui->comboBox_13->setCurrentText(adminInfoVector.at(4));//用户组
    ui->lineEdit_5->setText(adminInfoVector.at(5));//联系方式

    ui->comboBox_2->setCurrentText(adminInfoVector.at(6));//权限状态

    ui->dateTimeEdit->setDateTime(QDateTime::fromString(adminInfoVector.at(7),"yyyy/MM/dd hh:mm"));//权限起始时间
    ui->dateTimeEdit_2->setDateTime(QDateTime::fromString(adminInfoVector.at(8),"yyyy/MM/dd hh:mm"));//权限截止时间

    if(adminInfoVector.at(3) == "操作员")
    {
        ui->lineEdit->setEnabled(false);
        ui->comboBox->setEnabled(false);
        ui->comboBox_2->setEnabled(false);
        ui->dateTimeEdit->setEnabled(false);
        ui->dateTimeEdit_2->setEnabled(false);

        MysqlOperation::insertRecord(adminInfoVector.at(0),"","NNNNNN6N",tr("登录监控平台成功"));
    }
    else
    {
        MysqlOperation::insertRecord(adminInfoVector.at(0),"","NNNNNNN7",tr("登录监控平台成功"));
    }        
}

#define LOGIN 1
#define LOGIN_FAIL 2
#define ACCESS 3
#define NO_ACCESS 4
#define LOGOUT 5
#define GETLOCK_INFO 6
#define POWER_REQUEST 9
#define RECORD 10
void MainWindow::myslot_readMessage()
{
    QTcpSocket *clientSocket = (QTcpSocket*)sender();
    if(m_mapNetIsKey.contains(clientSocket))
    {//处理手机app通信
        QByteArray readBuf =  clientSocket->readAll();
        if(readBuf.isEmpty()||readBuf.isNull())
        {
            qDebug() << QObject::tr("读手机客户端数据为空");
            return;
        }
        qDebug() << QObject::tr("读手机客户端:") << readBuf.data();

        //处理json字符串
        QJsonParseError jsonError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(readBuf,&jsonError);
        if(jsonError.error != QJsonParseError::NoError)
        {
            qDebug() << QObject::tr("解析JSON字符串出错0");
            return;
        }

        if(jsonDoc.isObject())
        {
            QJsonObject jsonObj = jsonDoc.object();
            if(jsonObj.contains("cmd"))
            {
                QJsonValue jsonVal = jsonObj.take("cmd");
                switch(jsonVal.toVariant().toInt())
                {
                case LOGIN://手机端登录验证
                {
                    jsonVal = jsonObj.take("name");
                    QString logName = jsonVal.toString();
                    jsonVal = jsonObj.take("pswd");
                    QString password = jsonVal.toString();
                    qDebug() << QObject::tr("获取到用户名:%1 密码:%2").arg(logName).arg(password);

                    QVector<QString> adminInfoVector;
                    if(MysqlOperation::getAdminInfoByLogName(logName,adminInfoVector))
                    {
                        QVector<QString> empowerInfoVector;
                        empowerInfoVector.append(adminInfoVector.at(0));
                        empowerInfoVector.append(adminInfoVector.at(1));
                        empowerInfoVector.append(adminInfoVector.at(3));
                        empowerInfoVector.append(adminInfoVector.at(4));
                        empowerInfoVector.append(adminInfoVector.at(5));
                        empowerInfoVector.append(adminInfoVector.at(6));
                        empowerInfoVector.append(adminInfoVector.at(7));
                        empowerInfoVector.append(adminInfoVector.at(8));
                        MysqlOperation::insertEmpowerTable(empowerInfoVector);
                    }

                    int res = -1;
                    bool isPowerLong = false;
                    QString sTime,eTime;
                    res = MysqlOperation::isAdminInDatabase(logName, password);
                    if(res == 1)
                    {
                        res = MysqlOperation::isAdminEmpower(logName, password);
                        if(res == 1)
                        {
                            res = ACCESS;
                            isPowerLong = false;
                            MysqlOperation::getStEtBylogName(logName,sTime,eTime);
                        }
                        else if(res == 2)
                        {
                            res = ACCESS;
                            isPowerLong = true;
                            sTime = "0000/00/00 00:00";
                            eTime = "0000/00/00 00:00";
                        }
                        else
                        {
                            res = NO_ACCESS;
                        }
                    }
                    else
                    {
                        res = LOGIN_FAIL;
                    }

                    QJsonObject json;
                    json.insert("cmd", LOGIN);
                    json.insert("res", res);
                    if(res == ACCESS)
                    {
                        json.insert("st", sTime);
                        json.insert("et", eTime);
                    }

                    QJsonDocument document;
                    document.setObject(json);
                    QByteArray byteArray = document.toJson(QJsonDocument::Compact);

                    if(clientSocket->write(byteArray, byteArray.length()) != -1)
                    {
                        if(!clientSocket->waitForBytesWritten(WRITE_TIMEOUT))
                        {
                            qDebug() << QObject::tr("网络超时！");
                            return;
                        }

                        qDebug() << QObject::tr("发送成功:") + byteArray;

                        if(res == ACCESS)
                        {
                            m_mapNetIsKey.insert(clientSocket,logName);

                            QVector<QString> adminInfoVector;
                            if(MysqlOperation::getAdminInfoByLogName(logName,adminInfoVector))
                            {
                                int currentRow = ui->tableWidget_4->rowCount();

                                ui->tableWidget_4->insertRow(currentRow);
                               //姓名
                                QTableWidgetItem *item1 = new QTableWidgetItem(adminInfoVector.at(0));
                                //登录名
                                QTableWidgetItem *item2 = new QTableWidgetItem(logName);
                                //用户组
                                QTableWidgetItem *item3 = new QTableWidgetItem(adminInfoVector.at(4));

                                QTableWidgetItem *item4,*item5;
                                if(isPowerLong)
                                {
                                    item4 = new QTableWidgetItem(tr("该用户被永久授权"));
                                    item5 = new QTableWidgetItem(tr("该用户被永久授权"));
                                }
                                else
                                {
                                    //权限起始时间
                                    QDateTime sTime(QDateTime::fromString(adminInfoVector.at(7),"yyyy/MM/dd hh:mm"));
                                    item4 = new QTableWidgetItem(sTime.toString("yyyy-MM-dd hh:mm"));
                                    //权限截止时间
                                    QDateTime eTime(QDateTime::fromString(adminInfoVector.at(8),"yyyy/MM/dd hh:mm"));
                                    item5 = new QTableWidgetItem(eTime.toString("yyyy-MM-dd hh:mm"));
                                }

                                //登录时间
                                QTableWidgetItem *item6 = new QTableWidgetItem((QDateTime::currentDateTime())
                                                                               .toString("yyyy-MM-dd hh:mm"));
                                //联系方式
                                QTableWidgetItem *item7 = new QTableWidgetItem(adminInfoVector.at(5));

                                ui->tableWidget_4->setItem(currentRow,0,item1);
                                ui->tableWidget_4->setItem(currentRow,1,item2);
                                ui->tableWidget_4->setItem(currentRow,2,item3);
                                ui->tableWidget_4->setItem(currentRow,3,item4);
                                ui->tableWidget_4->setItem(currentRow,4,item5);
                                ui->tableWidget_4->setItem(currentRow,5,item6);
                                ui->tableWidget_4->setItem(currentRow,6,item7);

                                ui->tableWidget_4->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);

                                //记录日志
                                if(adminInfoVector.at(3) == tr("操作员"))
                                {
                                    MysqlOperation::insertRecord(adminInfoVector.at(0),"","NNNNN56N",tr("手机端app登录成功"));
                                }
                                else
                                {
                                    MysqlOperation::insertRecord(adminInfoVector.at(0),"","NNNNN5N7",tr("手机端app登录成功"));
                                }


                            }
                        }
                    }
                }
                    break;
                case GETLOCK_INFO://手机端获取设备信息
                {
                    QJsonArray jsonArray;
                    QJsonObject jsonObjInJsonArray;
                    QVector<QString> lockCodeVector;
                    QVector<QString> lockNameVector;
                    QVector<QString> lockAddrVector;
                    QVector<QString> lockJingDuVector;
                    QVector<QString> lockWeiDuVector;
                    MysqlOperation::getLockInfo(lockCodeVector,lockNameVector,lockAddrVector,
                                                lockJingDuVector,lockWeiDuVector);
                    for(int n = 0; n < lockCodeVector.length(); n++)
                    {
                        jsonObjInJsonArray.insert("id",lockCodeVector.at(n));
                        jsonObjInJsonArray.insert("name",lockNameVector.at(n));
                        jsonObjInJsonArray.insert("addr",lockAddrVector.at(n));
                        jsonObjInJsonArray.insert("lng",lockJingDuVector.at(n));
                        jsonObjInJsonArray.insert("lat",lockWeiDuVector.at(n));
                        jsonArray.append(jsonObjInJsonArray);
                    }
                    QJsonObject jsonObject;
                    jsonObject.insert("cmd", GETLOCK_INFO);
                    jsonObject.insert("data", jsonArray);

                    QJsonDocument document;
                    document.setObject(jsonObject);
                    QByteArray byteArray = document.toJson(QJsonDocument::Compact);

                    if(clientSocket->write(byteArray, byteArray.length()) != -1)
                    {
                        if(!clientSocket->waitForBytesWritten(WRITE_TIMEOUT))
                        {
                            qDebug() << QObject::tr("网络超时！");
                            return;
                        }

                        qDebug() << QObject::tr("发送成功:") + byteArray;
                    }
                }
                    break;
                case POWER_REQUEST:
                {
                    jsonVal = jsonObj.take("id");
                    QString lockCode = jsonVal.toString();
                    jsonVal = jsonObj.take("kc");
                    QString keyHexIP = jsonVal.toString();

                    QJsonObject jsonObject;
                    jsonObject.insert("cmd", POWER_REQUEST);
                    jsonObject.insert("mc", this->IP28hex("0.0.0.0"));
                    jsonObject.insert("ska", MysqlOperation::getSkaBykeyIP(this->hex82IP(keyHexIP)));

                    QJsonDocument document;
                    document.setObject(jsonObject);
                    QByteArray byteArray = document.toJson(QJsonDocument::Compact);

                    if(clientSocket->write(byteArray, byteArray.length()) != -1)
                    {
                        if(!clientSocket->waitForBytesWritten(WRITE_TIMEOUT))
                        {
                            qDebug() << QObject::tr("网络超时！");
                            return;
                        }

                        qDebug() << QObject::tr("发送成功:") + byteArray;

                        QVector<QString> adminInfoVector;
                        QString logName = m_mapNetIsKey.value(clientSocket);
                        if(MysqlOperation::getAdminInfoByLogName(logName,adminInfoVector))
                        {
                            //记录日志
                            if(adminInfoVector.at(3) == tr("操作员"))
                            {
                                MysqlOperation::insertRecord(adminInfoVector.at(0),"",
                                                             "0NNN4N6N",
                                                             tr("手机端app申请开编号为:%1的锁")
                                                             .arg(lockCode));
                            }
                            else
                            {
                                MysqlOperation::insertRecord(adminInfoVector.at(0),"",
                                                             "0NNN4NN7",
                                                             tr("手机端app申请开编号为:%1的锁")
                                                             .arg(lockCode));
                            }
                        }
                    }
                }
                    break;
                case RECORD:
                {
                    QString keyCode = MysqlOperation::getkeyCodeBykeyIP(this->hex82IP(jsonObj.take("kc").toString()));
                    QString logName = m_mapNetIsKey.value(clientSocket);
                    QString callName,phoneNum;
                    MysqlOperation::getCallNameBylogName(logName,callName,phoneNum);
                    if(MysqlOperation::isAdminRootBylogName(logName))
                    {
                        MysqlOperation::insertRecord(callName,keyCode,"0NNNNNN7",tr("开锁成功"));
                    }
                    else
                    {
                        MysqlOperation::insertRecord(callName,keyCode,"0NNNNN6N",tr("开锁成功"));
                    }

                }
                    break;
                default:
                    break;
                }
            }
        }
    }
}

void MainWindow::myslot_clientDisconnect()
{
    qDebug() << QObject::tr("手机客户端退出");
    m_currentConetCount--;
    this->currentNumLabel->setText(QObject::tr("当前连接数:<font color='red'>%1</font>")
                                   .arg(QString::number(m_currentConetCount)));
    QTcpSocket *clientSocket = (QTcpSocket*)sender();
    QString logName = m_mapNetIsKey.value(clientSocket);
    int rowCount = ui->tableWidget_4->rowCount();
    for(int row=0; row < rowCount; row++)
    {
        if(ui->tableWidget_4->item(row,1)->text().trimmed() == logName)
        {
            //记录日志
            QVector<QString> adminInfoVector;
            MysqlOperation::getAdminInfoByLogName(logName,adminInfoVector);
            if(adminInfoVector.at(3) == tr("操作员"))
            {
                MysqlOperation::insertRecord(adminInfoVector.at(0),"","NNNNN56N",tr("手机端app已退出"));
            }
            else
            {
                MysqlOperation::insertRecord(adminInfoVector.at(0),"","NNNNN5N7",tr("手机端app已退出"));
            }
            ui->tableWidget_4->removeRow(row);
            break;
        }
    }
    m_mapNetIsKey.remove(clientSocket);
}

void MainWindow::updateLockTableWidget()
{
    int rowCount = ui->tableWidget_2->rowCount();
    for(int row = 0; row < rowCount; row++)
    {
        ui->tableWidget_2->removeRow(0);
    }

    QSqlQuery sqlQuery;
    QString sqlStr = "select lockCode,lockName,lockAddr,userGroup,lockUser,lockIP from lockTable";
    sqlQuery.prepare(sqlStr);
    if(sqlQuery.exec())
    {
       //读取查询到的记录
       int row = 0;
       while(sqlQuery.next())
       {
           ui->tableWidget_2->insertRow(row);

           QCheckBox *checkBox = new QCheckBox;

           QTableWidgetItem *item1 = new QTableWidgetItem(sqlQuery.value(0).toString());

           QTableWidgetItem *item2 = new QTableWidgetItem(sqlQuery.value(1).toString());

           QTableWidgetItem *item3 = new QTableWidgetItem(sqlQuery.value(2).toString());

           QTableWidgetItem *item4 = new QTableWidgetItem(sqlQuery.value(3).toString());

           QString logStatus;
           if(m_mapDevCodeIsKey.contains(sqlQuery.value(0).toString()))
           {
               logStatus = "登录成功";
           }
           else
           {
               logStatus = "未登录";
           }
           QTableWidgetItem *item5 = new QTableWidgetItem(tr("%1").arg(logStatus));

           QTableWidgetItem *item6 = new QTableWidgetItem(tr("%1").arg(QString("未知")));

           if(MysqlOperation::isAdminRoot(ui->lineEdit_2->text().trimmed(),ui->lineEdit_3->text().trimmed()))
           {
               QTableWidgetItem *item7 = new QTableWidgetItem(sqlQuery.value(4).toString());
               ui->tableWidget_2->setItem(row, 7, item7);
               item7->setFlags(item7->flags() & (~Qt::ItemIsEditable));
               QTableWidgetItem *item8 = new QTableWidgetItem(sqlQuery.value(5).toString());
               item8->setFlags(item8->flags() & (~Qt::ItemIsEditable));
               ui->tableWidget_2->setItem(row, 8, item8);
           }


           item1->setFlags(item1->flags() & (~Qt::ItemIsEditable));
           item2->setFlags(item2->flags() & (~Qt::ItemIsEditable));
           item3->setFlags(item3->flags() & (~Qt::ItemIsEditable));
           item4->setFlags(item4->flags() & (~Qt::ItemIsEditable));
           item5->setFlags(item5->flags() & (~Qt::ItemIsEditable));
           item5->setFlags(item6->flags() & (~Qt::ItemIsEditable));

           ui->tableWidget_2->setCellWidget(row, 0, checkBox);
           ui->tableWidget_2->setItem(row, 1, item1);
           ui->tableWidget_2->setItem(row, 2, item2);
           ui->tableWidget_2->setItem(row, 3, item3);
           ui->tableWidget_2->setItem(row, 4, item4);
           ui->tableWidget_2->setItem(row, 5, item5);
           ui->tableWidget_2->setItem(row, 6, item6);

           row++;
       }
       ui->tableWidget_2->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
       qDebug() << QObject::tr("查询设备信息成功");

    }
    else
    {
        qDebug() << QObject::tr("查询设备信息失败:") << MysqlOperation::m_database.lastError();
    }
}

void MainWindow::myslot_updateStatusBarPort(int port)
{
    this->serPortLabel->setText(QObject::tr("端口号:<font color='red'>%1</font>")
                                .arg(QString::number(port)));
}
void MainWindow::myslot_updateStatusBarMaxCount(int maxCount)
{
    this->maxNumLabel->setText(QObject::tr("最大连接数:<font color='red'>%1</font>")
                               .arg(QString::number(maxCount)));
}

void MainWindow::initStatusBar()
{
    serIPLabel->setText(QObject::tr("服务器IP地址:") + "<font color='red'>" +
                        this->getLocalHostIP().toString() + "</font>");

    if(m_startServer)
    {
        serStatusLabel->setText(QObject::tr("服务器状态:<font color='red'>%1</font>")
                                .arg(QString("已启动")));
    }
    else
    {
        serStatusLabel->setText(QObject::tr("服务器状态:<font color='red'>%1</font>")
                                .arg(QString("未启动")));
    }

    currentNumLabel->setText(tr("当前连接数:<font color='red'>%1</font>")
                             .arg(QString::number(m_currentConetCount)));

    QStatusBar *statusbar = this->statusBar();

    statusbar->addWidget(serIPLabel);
    statusbar->addWidget(serPortLabel);
    statusbar->addWidget(serStatusLabel);
    statusbar->addWidget(currentNumLabel);
    statusbar->addWidget(maxNumLabel);
    statusbar->addWidget(m_progressBar);
}

void MainWindow::myslot_processConnection()
{
    QTcpSocket *clientSocket =this->m_listenSocket->nextPendingConnection();
    QString IPv4String = clientSocket->peerAddress().toString();
    qDebug() << QObject::tr("client IPv4Address:") << IPv4String;
    QString deviceCode = MysqlOperation::getLockCodeByIPv4(IPv4String);
    qDebug() << QObject::tr("deviceCode:") << deviceCode;

    m_currentConetCount++;
    this->currentNumLabel->setText(QObject::tr("当前连接数:<font color='red'>%1</font>")
                                   .arg(QString::number(m_currentConetCount)));

    if(deviceCode.isNull()||deviceCode.isEmpty())
    {
        this->m_mapNetIsKey.insert(clientSocket, "");
        connect(clientSocket,SIGNAL(readyRead()),this,SLOT(myslot_readMessage()));
        connect(clientSocket,SIGNAL(disconnected()),this,SLOT(myslot_clientDisconnect()));
    }
    else
    {
        this->m_mapDevCodeIsKey.insert(deviceCode, clientSocket);
        for(int row=0; row < ui->tableWidget_2->rowCount(); row++)
        {
            if(ui->tableWidget_2->item(row, 1)->text() == deviceCode)
            {
                ui->tableWidget_2->item(row, 5)->setText(QObject::tr("登录成功"));
            }
        }
    }
}

bool MainWindow::startServer()
{
    this->m_listenSocket = new QTcpServer;

    this->m_listenSocket->setMaxPendingConnections(ui->spinBox->value());

    QHostAddress serverIpAddr(ui->lineEdit_29->text().trimmed());
    if(this->m_listenSocket->listen(serverIpAddr,ui->spinBox_2->value()))
    {
       connect(this->m_listenSocket,SIGNAL(newConnection()),
               this,SLOT(myslot_processConnection()));

       if(ui->comboBox->currentText() == tr("操作员"))
       {
            MysqlOperation::insertRecord(ui->lineEdit->text().trimmed(),"","NNNNN56N",QObject::tr("启动服务器成功"));
       }
       else
       {
           MysqlOperation::insertRecord(ui->lineEdit->text().trimmed(),"","NNNNN5N7",QObject::tr("启动服务器成功"));
       }

       ui->pushButton_72->setText(QObject::tr("关闭服务器"));
       ui->pushButton_72->setIcon(QIcon(":/image/ball_on.ico"));
       serStatusLabel->setText(tr("服务器状态:<font color='red'>%1</font>").arg(QString("已启动")));
       m_startServer = true;
       return true;
    }
    else
    {
       ui->pushButton_72->setText(QObject::tr("开启服务器"));
       serStatusLabel->setText(tr("服务器状态:<font color='red'>%1</font>").arg(QString("启动失败")));
       m_startServer = false;
       return false;
    }
}

QHostAddress MainWindow::getLocalHostIP()
{
#if 0
    QList<QHostAddress> AddressList = QNetworkInterface::allAddresses();
    QHostAddress result;
    foreach(QHostAddress address, AddressList)
    {
        if(address.protocol() == QAbstractSocket::IPv4Protocol &&
           address != QHostAddress::Null &&
           address != QHostAddress::LocalHost)
        {
            if (address.toString().contains("127.0."))
            {
              continue;
            }
            result = address;
            break;
        }
    }
    return result;
#else
    QString localHostName = QHostInfo::localHostName();

    qDebug() <<"localHostName:"<<localHostName;
    QHostInfo info = QHostInfo::fromName(localHostName);
    QHostAddress result;
    foreach(QHostAddress address,info.addresses())
    {
        if(address.protocol() == QAbstractSocket::IPv4Protocol)
        {
            qDebug() <<"IPV4 Address: "<< address.toString();
            result = address;
            break;
        }
    }

    return result;
#endif
}

void MainWindow::autoScroll()
{
    QTextCursor cursor =  ((QTextBrowser*)sender())->textCursor();

    cursor.movePosition(QTextCursor::End);

    ((QTextBrowser*)sender())->setTextCursor(cursor);
}

QString MainWindow::IP28hex(QString IP)
{
    bool ok;
    if(IP.isEmpty()||IP.isNull())
    {
        qDebug() << "IP28hex fail";
        return NULL;
    }


    QString IPduan1;
    QString IPduan2;
    QString IPduan3;
    QString IPduan4;

    int n = 0;
    for(int i = 0; i < IP.length(); i++)
    {
        if(n == 0)
        {
            if(IP.at(i) == '.')
            {
                n = 1;
                continue;
            }
            else
            {
                IPduan1.append(IP.at(i));
            }
        }

        if(n == 1)
        {
            if(IP.at(i) == '.')
            {
                n = 2;
                continue;
            }
            else
            {
                IPduan2.append(IP.at(i));
            }
        }

        if(n == 2)
        {
            if(IP.at(i) == '.')
            {
                n = 3;
                continue;
            }
            else
            {
                IPduan3.append(IP.at(i));
            }
        }

        if(n == 3)
        {

            IPduan4.append(IP.at(i));

        }
    }

    int duan1 = IPduan1.toInt(&ok, 10);
    if(!ok)
    {
        return NULL;
    }
    int duan2 = IPduan2.toInt(&ok, 10);
    if(!ok)
    {
        return NULL;
    }
    int duan3 = IPduan3.toInt(&ok, 10);
    if(!ok)
    {
        return NULL;
    }
    int duan4 = IPduan4.toInt(&ok, 10);
    if(!ok)
    {
        return NULL;
    }

    QString devAddress;
    devAddress.sprintf("%02X%02X%02X%02X",
                     duan1 & 0xff,
                     duan2 & 0xff,
                     duan3 & 0xff,
                     duan4 & 0xff);
    qDebug() << "devAddress:" << devAddress;
    return devAddress;
}

int MainWindow::getKeyEmpowerStatus(QString IP)
{
    bool ok;
    if(IP.isEmpty()||IP.isNull())
    {
        qDebug() << "IP28hex fail";
        return -1;
    }


    QString IPduan1;
    QString IPduan2;
    QString IPduan3;
    QString IPduan4;

    int n = 0;
    for(int i = 0; i < IP.length(); i++)
    {
        if(n == 0)
        {
            if(IP.at(i) == '.')
            {
                n = 1;
                continue;
            }
            else
            {
                IPduan1.append(IP.at(i));
            }
        }

        if(n == 1)
        {
            if(IP.at(i) == '.')
            {
                n = 2;
                continue;
            }
            else
            {
                IPduan2.append(IP.at(i));
            }
        }

        if(n == 2)
        {
            if(IP.at(i) == '.')
            {
                n = 3;
                continue;
            }
            else
            {
                IPduan3.append(IP.at(i));
            }
        }

        if(n == 3)
        {

            IPduan4.append(IP.at(i));

        }
    }

    int duan1 = IPduan1.toInt(&ok, 10);
    if(!ok)
    {
        return -1;
    }
    int duan2 = IPduan2.toInt(&ok, 10);
    if(!ok)
    {
        return -1;
    }
    int duan3 = IPduan3.toInt(&ok, 10);
    if(!ok)
    {
        return -1;
    }
    int duan4 = IPduan4.toInt(&ok, 10);
    if(!ok)
    {
        return -1;
    }

    if((duan1 != 0)&&(duan2 != 0)&&(duan3 != 0)&&(duan4 != 0))
    {
        return 0;
    }
    else if((duan1 != 0)&&(duan2 != 0)&&(duan3 != 0)&&(duan4 == 0))
    {
        return 1;
    }
    else if((duan1 != 0)&&(duan2 != 0)&&(duan3 == 0)&&(duan4 == 0))
    {
        return 2;
    }
    else if((duan1 != 0)&&(duan2 == 0)&&(duan3 == 0)&&(duan4 == 0))
    {
        return 3;
    }
    else if((duan1 == 0)&&(duan2 == 0)&&(duan3 == 0)&&(duan4 == 0))
    {
        return 4;
    }
    else
    {
        return -1;
    }
}

QString MainWindow::hex82IP(QString hex8)
{
    QString IPduan1;
    QString IPduan2;
    QString IPduan3;
    QString IPduan4;

    bool ok;

    IPduan1 = QString::number(hex8.left(2).toInt(&ok, 16), 10);
    if(!ok)
    {
        qDebug() << "hex82IP fail";
        return NULL;
    }
    IPduan2 = QString::number(hex8.mid(2, 2).toInt(&ok, 16), 10);
    if(!ok)
    {
        qDebug() << "hex82IP fail";
        return NULL;
    }
    IPduan3 = QString::number(hex8.mid(4, 2).toInt(&ok, 16), 10);
    if(!ok)
    {
        qDebug() << "hex82IP fail";
        return NULL;
    }
    IPduan4 = QString::number(hex8.mid(6 ,2).toInt(&ok, 16), 10);
    if(!ok)
    {
        qDebug() << "hex82IP fail";
        return NULL;
    }

    qDebug() << "设备编码IP形式:" <<IPduan1 + "." + IPduan2 + "." + IPduan3 + "." + IPduan4;

    return IPduan1 + "." + IPduan2 + "." + IPduan3 + "." + IPduan4;
}

bool MainWindow::getKeyAddress(QString & keyAddress)
{
    char *pchStr = new char[128];
    char *chkSum = new char;
    sprintf_s(pchStr, 128, "FFFFFFFFKC,");
    SPComm::getChksum(pchStr, chkSum);
    sprintf_s(pchStr, 128, "#FFFFFFFFKC,%02X\r", *chkSum & 0xff);

    //写串口
    qDebug() << tr("串口发送:") <<pchStr;
    ui->textBrowser->append(tr("发送:") + pchStr);
    int retWrite = SPComm::writeData(pchStr, strlen(pchStr));
    if(retWrite < 0)
    {
        delete[] pchStr;
        pchStr = NULL;
        delete chkSum;
        chkSum = NULL;

        return false;
    }

    //读串口
    bool retRead = SPComm::readAllData(pchStr, 1500);
    if(retRead)
    {
        qDebug() << tr("串口接收:") << pchStr;
        ui->textBrowser->append(tr("接收:") + pchStr);
        QString readStr = pchStr;
        if(!readStr.contains("??"))
        {
            keyAddress = readStr.mid(11, 8);
            qDebug() << "钥匙地址为:" << keyAddress;

            int ret = MysqlOperation::iskeyIpInDatabase(this->hex82IP(keyAddress));
            if(ret == 0)
            {
#ifdef USE_SELF_MSGBOX
                int ok = myHelper::ShowMessageBoxQuesion(QObject::tr("发现新钥匙，是否添加到数据库？"));
                if(ok == 1)
#else
                int ok = QMessageBox::information(NULL, QObject::tr("提示"),
                                              QObject::tr("发现新钥匙，是否添加到数据库？"),
                                              QMessageBox::Yes | QMessageBox::No,
                                              QMessageBox::Yes);
                if(ok == QMessageBox::Yes)
#endif
                {
                    QVector<QString> keyVector;
                    keyVector.append("");//0
                    keyVector.append("");//1
                    keyVector.append("");//2
                    keyVector.append("");//3
                    keyVector.append("");//4
                    keyVector.append("");//5
                    keyVector.append("");//6
                    keyVector.append("");//7
                    keyVector.append("");//8
                    keyVector.append(this->hex82IP(keyAddress));
                    if(MysqlOperation::insertIntoKeyTable(keyVector))
                    {
#ifdef USE_SELF_MSGBOX
                        myHelper::ShowMessageBoxInfo(QObject::tr("添加新钥匙成功！"));
#else
                        QMessageBox::information(NULL, QObject::tr("提示"),
                                                 QObject::tr("添加新钥匙成功！"),
                                                 QMessageBox::Yes);
#endif
                        this->updateKeyTableView();
                    }
                    else
                    {
#ifdef USE_SELF_MSGBOX
                        myHelper::ShowMessageBoxError(QObject::tr("添加新钥匙失败！"));
#else
                        QMessageBox::warning(NULL, QObject::tr("警告"),
                                                 QObject::tr("添加新钥匙失败！"),
                                                 QMessageBox::Yes);
#endif
                    }
                }
            }

            delete[] pchStr;
            pchStr = NULL;
            delete chkSum;
            chkSum = NULL;

            return true;
        }

    }

#ifdef USE_SELF_MSGBOX
    myHelper::ShowMessageBoxError(QObject::tr("获取钥匙地址失败"));
#else
    QMessageBox::warning(NULL, QObject::tr("提示"),
                             QObject::tr("获取钥匙地址失败"),
                             QMessageBox::Yes);
#endif

    delete[] pchStr;
    pchStr = NULL;
    delete chkSum;
    chkSum = NULL;

    return false;
}

void MainWindow::updatePortNameComboBox()
{
    ui->comboBox_9->clear();
    QVector<QString> comVector;
    SPComm::getAvailablePortName(comVector);
    for(int i = comVector.size() - 1; i >= 0; i--)
    {
        ui->comboBox_9->insertItem(i,comVector.at(i));
    }
}

void MainWindow::updateLoadEmpowerTableView()
{
    ui->dateTimeEdit_3->setDateTime(QDateTime::currentDateTime());

    m_loadEmpowerModel.setTable("empowerTable");
    m_loadEmpowerModel.setEditStrategy(QSqlTableModel::OnManualSubmit);
    m_loadEmpowerModel.select();
    m_loadEmpowerModel.setHeaderData(1, Qt::Horizontal, QObject::tr("姓名"));
    m_loadEmpowerModel.setHeaderData(2, Qt::Horizontal, QObject::tr("登录名"));
    m_loadEmpowerModel.setHeaderData(3, Qt::Horizontal, QObject::tr("权限组"));
    m_loadEmpowerModel.setHeaderData(4, Qt::Horizontal, QObject::tr("用户组"));
    m_loadEmpowerModel.setHeaderData(5, Qt::Horizontal, QObject::tr("联系方式"));
    m_loadEmpowerModel.setHeaderData(6, Qt::Horizontal, QObject::tr("权限状态"));
    m_loadEmpowerModel.setHeaderData(7, Qt::Horizontal, QObject::tr("权限起始时间"));
    m_loadEmpowerModel.setHeaderData(8, Qt::Horizontal, QObject::tr("权限截止时间"));
    m_loadEmpowerModel.setHeaderData(9, Qt::Horizontal, QObject::tr("下载票据时间"));

    ui->tableView_4->setModel(&m_loadEmpowerModel);
    ui->tableView_4->hideColumn(0);
    ui->tableView_4->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

void MainWindow::updateEmpowerTableView()
{
    m_empowerModel.setTable("adminTable");
    m_empowerModel.setEditStrategy(QSqlTableModel::OnManualSubmit);
    m_empowerModel.select();
    m_empowerModel.setHeaderData(1, Qt::Horizontal, QObject::tr("姓名"));
    m_empowerModel.setHeaderData(2, Qt::Horizontal, QObject::tr("登录名"));
    m_empowerModel.setHeaderData(4, Qt::Horizontal, QObject::tr("权限组"));
    m_empowerModel.setHeaderData(5, Qt::Horizontal, QObject::tr("用户组"));
    m_empowerModel.setHeaderData(6, Qt::Horizontal, QObject::tr("联系方式"));
    m_empowerModel.setHeaderData(7, Qt::Horizontal, QObject::tr("权限状态"));
    m_empowerModel.setHeaderData(8, Qt::Horizontal, QObject::tr("权限起始时间"));
    m_empowerModel.setHeaderData(9, Qt::Horizontal, QObject::tr("权限截止时间"));

    QVector<QString> vector4;
    vector4.append(tr("操作员"));
    vector4.append(tr("管理员"));
    QComboBoxDelegate *comboBoxDelegate4 = new QComboBoxDelegate(vector4);
    ui->tableView->setItemDelegateForColumn(4, comboBoxDelegate4);

    QVector<QString> vector5;
    MysqlOperation::getGroupInfo(vector5);
    QComboBoxDelegate *comboBoxDelegate5 = new QComboBoxDelegate(vector5);
    ui->tableView->setItemDelegateForColumn(5, comboBoxDelegate5);

    QVector<QString> vector7;
    vector7.append(tr("未授权"));
    vector7.append(tr("期间授权"));
    vector7.append(tr("永久授权"));
    vector7.append(tr("权限过期"));
    vector7.append(tr("权限未到"));
    vector7.append(tr("列入黑名单"));
    QComboBoxDelegate *comboBoxDelegate7 = new QComboBoxDelegate(vector7);
    ui->tableView->setItemDelegateForColumn(7, comboBoxDelegate7);
    ui->tableView->setModel(&m_empowerModel);
    ui->tableView->hideColumn(0);
    ui->tableView->hideColumn(3);
    ui->tableView->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

void MainWindow::updateKeyTableView()
{
    m_keyModel.setTable("keyTable");
    m_keyModel.setEditStrategy(QSqlTableModel::OnManualSubmit);
    m_keyModel.select();
    m_keyModel.setHeaderData(1, Qt::Horizontal, QObject::tr("钥匙编号"));
    m_keyModel.setHeaderData(2, Qt::Horizontal, QObject::tr("钥匙名称"));
    m_keyModel.setHeaderData(3, Qt::Horizontal, QObject::tr("所属用户组"));
    m_keyModel.setHeaderData(4, Qt::Horizontal, QObject::tr("A级密钥"));
    m_keyModel.setHeaderData(5, Qt::Horizontal, QObject::tr("B级密钥"));
    m_keyModel.setHeaderData(6, Qt::Horizontal, QObject::tr("权限状态"));
    m_keyModel.setHeaderData(7, Qt::Horizontal, QObject::tr("权限掩码"));
    m_keyModel.setHeaderData(8, Qt::Horizontal, QObject::tr("权限起始日期"));
    m_keyModel.setHeaderData(9, Qt::Horizontal, QObject::tr("权限截止日期"));
    int ret = MysqlOperation::isAdminRoot(ui->lineEdit_2->text().trimmed(),ui->lineEdit_3->text().trimmed());
    if(ret == 1)
    {
       m_keyModel.setHeaderData(10, Qt::Horizontal, QObject::tr("钥匙ID"));
    }
    QVector<QString> vector3;
    MysqlOperation::getGroupInfo(vector3);
    QComboBoxDelegate *comboBoxDelegate3 = new QComboBoxDelegate(vector3);
    ui->tableView_7->setItemDelegateForColumn(3, comboBoxDelegate3);

    QVector<QString> vector6;
    vector6.append(tr("1开1"));
    vector6.append(tr("1开254"));
    vector6.append(tr("1开254^2"));
    vector6.append(tr("1开254^3"));
    vector6.append(tr("全开"));
    vector6.append(tr("权限过期"));
    vector6.append(tr("权限未到"));
    QComboBoxDelegate *comboBoxDelegate6 = new QComboBoxDelegate(vector6);
    ui->tableView_7->setItemDelegateForColumn(6, comboBoxDelegate6);

    ui->tableView_7->setModel(&m_keyModel);
    ui->tableView_7->hideColumn(0);
    if(ret != 1)
    {
        ui->tableView_7->hideColumn(4);
        ui->tableView_7->hideColumn(5);
        ui->tableView_7->hideColumn(10);
    }

    ui->tableView_7->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

void MainWindow::updateLockTableView()
{
    m_lockModel.setTable("lockTable");
    m_lockModel.setEditStrategy(QSqlTableModel::OnManualSubmit);
    m_lockModel.select();
    m_lockModel.setHeaderData(1, Qt::Horizontal, QObject::tr("锁编号"));
    m_lockModel.setHeaderData(2, Qt::Horizontal, QObject::tr("锁名称"));
    m_lockModel.setHeaderData(3, Qt::Horizontal, QObject::tr("所属用户组"));
    m_lockModel.setHeaderData(4, Qt::Horizontal, QObject::tr("地理位置"));
    m_lockModel.setHeaderData(5, Qt::Horizontal, QObject::tr("经度"));
    m_lockModel.setHeaderData(6, Qt::Horizontal, QObject::tr("纬度"));
    m_lockModel.setHeaderData(7, Qt::Horizontal, QObject::tr("ip地址"));
    int ret = MysqlOperation::isAdminRoot(ui->lineEdit_2->text().trimmed(),ui->lineEdit_3->text().trimmed());
    if(ret == 1)
    {
        m_lockModel.setHeaderData(8, Qt::Horizontal, QObject::tr("锁用户"));
    }
    m_lockModel.setHeaderData(9, Qt::Horizontal, QObject::tr("服役时间"));
    if(ret == 1)
    {
        m_lockModel.setHeaderData(10, Qt::Horizontal, QObject::tr("锁ID"));
    }

    QVector<QString> vector3;
    MysqlOperation::getGroupInfo(vector3);
    QComboBoxDelegate *comboBoxDelegate3 = new QComboBoxDelegate(vector3);
    ui->tableView_5->setItemDelegateForColumn(3, comboBoxDelegate3);

    ui->tableView_5->setModel(&m_lockModel);
    ui->tableView_5->hideColumn(0);
    if(ret != 1)
    {
        ui->tableView_5->hideColumn(8);
        ui->tableView_5->hideColumn(10);
    }

    ui->tableView_5->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

void MainWindow::updateGroupComboBox()
{
    ui->comboBox_8->clear();
    ui->comboBox_6->clear();
    ui->comboBox_7->clear();
    ui->comboBox_13->clear();

    QVector<QString> groupVector;
    MysqlOperation::getGroupInfo(groupVector);
    for(int i=0; i < groupVector.size(); i++)
    {
        ui->comboBox_8->insertItem(i, tr("%1").arg(groupVector.at(i)));
        ui->comboBox_6->insertItem(i, tr("%1").arg(groupVector.at(i)));
        ui->comboBox_7->insertItem(i, tr("%1").arg(groupVector.at(i)));
        ui->comboBox_13->insertItem(i, tr("%1").arg(groupVector.at(i)));
    }
}

void MainWindow::updateAdminTableView()
{
    m_adminModel.setTable("adminTable");
    m_adminModel.setEditStrategy(QSqlTableModel::OnManualSubmit);
    m_adminModel.select();
    m_adminModel.setHeaderData(1, Qt::Horizontal, QObject::tr("姓名"));
    m_adminModel.setHeaderData(2, Qt::Horizontal, QObject::tr("登录名"));
    m_adminModel.setHeaderData(3, Qt::Horizontal, QObject::tr("密码"));
    m_adminModel.setHeaderData(4, Qt::Horizontal, QObject::tr("权限组"));
    m_adminModel.setHeaderData(5, Qt::Horizontal, QObject::tr("用户组"));
    m_adminModel.setHeaderData(6, Qt::Horizontal, QObject::tr("联系方式"));
    m_adminModel.setHeaderData(7, Qt::Horizontal, QObject::tr("权限状态"));
    m_adminModel.setHeaderData(8, Qt::Horizontal, QObject::tr("权限起始时间"));
    m_adminModel.setHeaderData(9, Qt::Horizontal, QObject::tr("权限截止时间"));

    QVector<QString> vector4;
    vector4.append(tr("操作员"));
    vector4.append(tr("管理员"));
    QComboBoxDelegate *comboBoxDelegate4 = new QComboBoxDelegate(vector4);
    ui->tableView_2->setItemDelegateForColumn(4, comboBoxDelegate4);

    QVector<QString> vector5;
    MysqlOperation::getGroupInfo(vector5);
    QComboBoxDelegate *comboBoxDelegate5 = new QComboBoxDelegate(vector5);
    ui->tableView_2->setItemDelegateForColumn(5, comboBoxDelegate5);

    QVector<QString> vector7;
    vector7.append(tr("未授权"));
    vector7.append(tr("期间授权"));
    vector7.append(tr("永久授权"));
    vector7.append(tr("权限过期"));
    vector7.append(tr("权限未到"));
    vector7.append(tr("列入黑名单"));
    QComboBoxDelegate *comboBoxDelegate7 = new QComboBoxDelegate(vector7);
    ui->tableView_2->setItemDelegateForColumn(7, comboBoxDelegate7);
    ui->tableView_2->setModel(&m_adminModel);
    ui->tableView_2->hideColumn(0);
    ui->tableView_2->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}
void MainWindow::updateGroupTableView()
{
    m_groupModel.setTable("groupTable");
    m_groupModel.setEditStrategy(QSqlTableModel::OnManualSubmit);
    m_groupModel.select();

    m_groupModel.setHeaderData(1, Qt::Horizontal, QObject::tr("用户组名"));
    m_groupModel.setHeaderData(2, Qt::Horizontal, QObject::tr("创建时间"));

    ui->tableView_6->setModel(&m_groupModel);
    ui->tableView_6->hideColumn(0);
}

void MainWindow::setCurrentStackPageSlot(int index)
{
    QString itemText = ui->listWidget->item(index)->text();

    if(itemText == QObject::tr("个人信息"))
    {
        ui->stackedWidget->setCurrentIndex(0);
    }
    else if(itemText == QObject::tr("地图显示"))
    {
        ui->stackedWidget->setCurrentIndex(1);

        //加载本地的html文件
        QString dirStr = QDir::currentPath();
        qDebug() << "\ncurrentPath:" << dirStr;
        dirStr.replace("/", "\\");
        QUrl url("file:///" + dirStr + "\\baidumap.html");
        ui->webView->setUrl(url);

    }
    else if(itemText == QObject::tr("授权管理"))
    {
        ui->stackedWidget->setCurrentIndex(2);
    }
    else if(itemText == QObject::tr("用户管理"))
    {
        ui->stackedWidget->setCurrentIndex(3);
        this->updateAdminTableView();
    }
    else if(itemText == QObject::tr("锁具管理"))
    {
        ui->stackedWidget->setCurrentIndex(4);
    }
    else if(itemText == QObject::tr("钥匙管理"))
    {
        ui->stackedWidget->setCurrentIndex(5);
    }
    else if(itemText == QObject::tr("日志管理"))
    {
        ui->stackedWidget->setCurrentIndex(6);
    }
    else if(itemText == QObject::tr("串口设置"))
    {
        ui->stackedWidget->setCurrentIndex(7);
    }
    else if(itemText == QObject::tr("服务器管理"))
    {
        ui->stackedWidget->setCurrentIndex(8);
    }
    else
    {
        ui->stackedWidget->setCurrentIndex(1);
    }
}

void MainWindow::on_pushButton_6_clicked()
{
    int rowNum = m_adminModel.rowCount();
    m_adminModel.insertRow(rowNum); //添加一行

    QDateTime currentDateTime = QDateTime::currentDateTime();
    m_adminModel.setData(m_adminModel.index(rowNum, 8), currentDateTime);
    m_adminModel.setData(m_adminModel.index(rowNum, 9), currentDateTime);
}

void MainWindow::on_pushButton_45_clicked()
{
    ui->stackedWidget_2->setCurrentIndex(0);
}

void MainWindow::on_pushButton_47_clicked()
{
    ui->stackedWidget_2->setCurrentIndex(1);
}

void MainWindow::on_pushButton_46_clicked()
{
    ui->stackedWidget_2->setCurrentIndex(2);
}

void MainWindow::on_pushButton_48_clicked()
{
    ui->stackedWidget_2->setCurrentIndex(3);
}

void MainWindow::on_pushButton_49_clicked()
{       
    QString KEY_ADDRESS;
    if(!this->getKeyAddress(KEY_ADDRESS))
    {
        return;
    }

    QString password = "100000";
    char *pchStr = new char[128];
    char *chkSum = new char;
    sprintf_s(pchStr, 128, "%sG%s,", KEY_ADDRESS.toLocal8Bit().data(), password.toLocal8Bit().data());
    SPComm::getChksum(pchStr, chkSum);
    sprintf_s(pchStr, 128, "#%sG%s,%02X\r\n", KEY_ADDRESS.toLocal8Bit().data(),
              password.toLocal8Bit().data(), *chkSum & 0xff);
    //写串口
    qDebug() << "串口发送:" <<pchStr;

#ifndef SHOW_COM_DATA
    ui->textBrowser->append(tr("发送:注销钥匙操作权限指令"));
#else
    ui->textBrowser->append(tr("发送:") + pchStr);
#endif

    int retWrite = SPComm::writeData(pchStr, strlen(pchStr));
    if(retWrite < 0)
    {
        delete[] pchStr;
        pchStr = NULL;
        delete chkSum;
        chkSum = NULL;

        return;
    }

    //读串口
    bool retRead = SPComm::readAllData(pchStr, 1500);
    if(retRead)
    {
        qDebug() << "串口接收:" <<pchStr;
        QString keyRet = pchStr;

        if(keyRet.contains("??"))
        {
#ifndef SHOW_COM_DATA
    ui->textBrowser->append(tr("接收:注销钥匙操作权限成功"));
#else
    ui->textBrowser->append(tr("接收:") + pchStr);
#endif

#ifdef USE_SELF_MSGBOX
            myHelper::ShowMessageBoxInfo(QObject::tr("注销权限成功"));
#else
            QMessageBox::information(NULL, QObject::tr("提示"),
                                     QObject::tr("注销权限成功"),
                                     QMessageBox::Yes);
#endif

            //数据库操作
            this->updateKeyTableView();

            if(ui->comboBox->currentText() == tr("管理员"))
            {
                MysqlOperation::insertRecord(ui->lineEdit->text(),
                                             MysqlOperation::getkeyCodeBykeyIP(hex82IP(KEY_ADDRESS)),
                                             "NNN3NN6N",tr("注销钥匙操作权限成功"));
            }
            else
            {
                MysqlOperation::insertRecord(ui->lineEdit->text(),
                                             MysqlOperation::getkeyCodeBykeyIP(hex82IP(KEY_ADDRESS)),
                                             "NNN3NNN7",tr("注销钥匙操作权限成功"));
            }


            delete[] pchStr;
            pchStr = NULL;
            delete chkSum;
            chkSum = NULL;

            return;
        }
    }

#ifndef SHOW_COM_DATA
    ui->textBrowser->append(tr("接收:注销钥匙操作权限失败"));
#else
    ui->textBrowser->append(tr("接收:") + pchStr);
#endif

#ifdef USE_SELF_MSGBOX
    myHelper::ShowMessageBoxError(QObject::tr("注销权限失败"));
#else
    QMessageBox::warning(NULL, QObject::tr("提示"),
                             QObject::tr("注销权限失败"),
                             QMessageBox::Yes);
#endif

    delete[] pchStr;
    pchStr = NULL;
    delete chkSum;
    chkSum = NULL;

    return;
}

void MainWindow::on_pushButton_50_clicked()
{
    ui->stackedWidget_2->setCurrentIndex(4);
    ui->dateEdit->setDate(QDate::currentDate());
    ui->dateEdit_2->setDate(QDate::currentDate());
}

void MainWindow::on_pushButton_55_clicked()
{
    ui->stackedWidget_2->setCurrentIndex(5);
    QString KEY_ADDRESS;
    this->getKeyAddress(KEY_ADDRESS);
    ui->lineEdit_32->setText(this->hex82IP(KEY_ADDRESS));
}

void MainWindow::on_pushButton_56_clicked()
{
    ui->stackedWidget_2->setCurrentIndex(6);

    int rowCount = ui->tableWidget->rowCount();
    for(int i = 0; i < rowCount; i++)
    {
        ui->tableWidget->removeRow(0);
    }

    QString KEY_ADDRESS;
    if(!this->getKeyAddress(KEY_ADDRESS))
    {
        return;
    }

    char *pchStr = new char[128];
    char *chkSum = new char;
    sprintf_s(pchStr, 128, "%sRLKN,", KEY_ADDRESS.toLocal8Bit().data());
    SPComm::getChksum(pchStr, chkSum);
    sprintf_s(pchStr, 128, "#%sRLKN,%02X\r", KEY_ADDRESS.toLocal8Bit().data(), *chkSum & 0xff);
    //写串口
    qDebug() << "串口发送:" <<pchStr;
#ifndef SHOW_COM_DATA
    ui->textBrowser->append(tr("发送:获取钥匙开锁记录总数指令"));
#else
    ui->textBrowser->append(tr("发送:") + pchStr);
#endif
    int retWrite = SPComm::writeData(pchStr, strlen(pchStr));
    if(retWrite < 0)
    {
        delete[] pchStr;
        pchStr = NULL;
        delete chkSum;
        chkSum = NULL;

        return;
    }

    //读串口
    bool retRead = SPComm::readAllData(pchStr, 1500);
    if(retRead)
    {
        qDebug() << "串口接收:" <<pchStr;
        QString retKeyOpenInfo = pchStr;

        if(!retKeyOpenInfo.contains("??"))
        {
            bool ok;
            int openCount = -1;

            openCount = retKeyOpenInfo.mid(12, retKeyOpenInfo.indexOf(",") - 12).toInt(&ok, 10);
            if(!ok)
            {
                openCount = 0;
            }
            if(openCount < 1)
            {
#ifndef SHOW_COM_DATA
    ui->textBrowser->append(tr("接收:获取钥匙开锁记录总数0或失败"));
#else
    ui->textBrowser->append(tr("接收:") + pchStr);
#endif

#ifdef USE_SELF_MSGBOX
    myHelper::ShowMessageBoxInfo(QObject::tr("获取到开锁记录总数为0"));
#else
                QMessageBox::information(NULL, QObject::tr("提示"),
                                         QObject::tr("获取到开锁记录总数为0"),
                                         QMessageBox::Yes);
#endif
            }
            else
            {
                int count = -1;
                if(openCount < 50)
                {
                    count = openCount;
                }
                else
                {
                    count = 50;
                }

                for(int i = 1; i <= count; i++)
                {
                    sprintf_s(pchStr, 128, "%sRLK%03d,", KEY_ADDRESS.toLocal8Bit().data(), i);
                    SPComm::getChksum(pchStr, chkSum);
                    sprintf_s(pchStr, 128, "#%sRLK%03d,%02X\r", KEY_ADDRESS.toLocal8Bit().data(), i, *chkSum & 0xff);
                    //写串口
                    qDebug() << "串口发送:" <<pchStr;
                    int retWrite = SPComm::writeData(pchStr, strlen(pchStr));
                    if(retWrite < 0)
                    {
                        delete[] pchStr;
                        pchStr = NULL;
                        delete chkSum;
                        chkSum = NULL;

                        return;
                    }
                    //读串口
                    bool retRead = SPComm::readAllData(pchStr, 1500);
                    if(retRead)
                    {
                        qDebug() << "串口接收:" <<pchStr;
                        QString retKeyOpenInfo = pchStr;

                        if(!retKeyOpenInfo.contains("??"))
                        {
                            //显示开锁记录
                            int index = retKeyOpenInfo.indexOf(",");
                            QString recordLC = retKeyOpenInfo.mid(12, 8);
                            QString recordTime = retKeyOpenInfo.mid(index + 1,
                                                                 retKeyOpenInfo.lastIndexOf(",") - index - 1);
                            QString lockCode = this->hex82IP(recordLC);
                            QString lockName;
                            QString userGroup;
                            QString lockAddr;
                            MysqlOperation::getLockNameAddrGroupByCode(lockCode,lockName,lockAddr,userGroup);

                            //newItem0
                            QTableWidgetItem *newItem0 = new QTableWidgetItem(lockCode);
                            newItem0->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

                            //newItem1
                            QTableWidgetItem *newItem1 = new QTableWidgetItem(lockName);
                            newItem1->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

                            //newItem2
                            QTableWidgetItem *newItem2 = new QTableWidgetItem(userGroup);
                            newItem2->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

                            //newItem3
                            QTableWidgetItem *newItem3 = new QTableWidgetItem(lockAddr);
                            newItem3->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

                            //newItem4
                            QTableWidgetItem *newItem4 = new QTableWidgetItem(
                                        "于" + recordTime.left(4) +
                                        "-" + recordTime.mid(4, 2) +
                                        "-" + recordTime.mid(6, 2) +
                                        ", " + recordTime.mid(8, 2) +
                                        ":" + recordTime.mid(10, 2) + "开锁成功");
                            newItem4->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);


                            ui->tableWidget->insertRow(i-1);
                            ui->tableWidget->setItem(i-1, 0, newItem0);
                            ui->tableWidget->setItem(i-1, 1, newItem1);
                            ui->tableWidget->setItem(i-1, 2, newItem2);
                            ui->tableWidget->setItem(i-1, 3, newItem3);
                            ui->tableWidget->setItem(i-1, 4, newItem4);

#ifndef SHOW_COM_DATA
    ui->textBrowser->append(tr("接收:获取钥匙开锁记录成功"));
#else
    ui->textBrowser->append(tr("接收:") + pchStr);
#endif

                            //记录日志

                            MysqlOperation::insertRecord("",
                                                         MysqlOperation::getkeyCodeBykeyIP(hex82IP((KEY_ADDRESS))),
                                                         "N1N3NNNN",tr("开锁%1成功，时间:%2")
                                                         .arg(lockCode)
                                                         .arg(recordTime));
                        }
                        else
                        {
#ifdef USE_SELF_MSGBOX
                            myHelper::ShowMessageBoxError(QObject::tr("获取钥匙开锁记录失败"));
#else
                            QMessageBox::warning(NULL, QObject::tr("提示"),
                                                     QObject::tr("获取钥匙开锁记录失败"),
                                                     QMessageBox::Yes);
#endif
                            delete[] pchStr;
                            pchStr = NULL;
                            delete chkSum;
                            chkSum = NULL;

                            return;
                        }
                    }
                }

                //记录日志
                if(ui->comboBox->currentText() == tr("管理员"))
                {
                    MysqlOperation::insertRecord(ui->lineEdit->text(),
                                                 MysqlOperation::getkeyCodeBykeyIP(hex82IP(KEY_ADDRESS)),
                                                 "NNN3NN6N",tr("查看了钥匙的开锁日志"));
                }
                else
                {
                    MysqlOperation::insertRecord(ui->lineEdit->text(),
                                                 MysqlOperation::getkeyCodeBykeyIP(hex82IP(KEY_ADDRESS)),
                                                 "NNN3NNN7",tr("查看了钥匙的开锁日志"));
                }
            }
        }
        else
        {
#ifndef SHOW_COM_DATA
    ui->textBrowser->append(tr("接收:获取钥匙开锁记录是失败"));
#else
    ui->textBrowser->append(tr("接收:") + pchStr);
#endif

#ifdef USE_SELF_MSGBOX
    myHelper::ShowMessageBoxError(QObject::tr("获取钥匙开锁记录总数失败"));
#else
            QMessageBox::warning(NULL, QObject::tr("提示"),
                                     QObject::tr("获取钥匙开锁记录总数失败"),
                                     QMessageBox::Yes);
#endif
        }
    }

    delete[] pchStr;
    pchStr = NULL;
    delete chkSum;
    chkSum = NULL;

    return;
}

void MainWindow::on_pushButton_30_clicked()
{
    ui->stackedWidget_3->setCurrentIndex(0);
}

void MainWindow::on_pushButton_29_clicked()
{
    ui->stackedWidget_3->setCurrentIndex(1);

    if(!m_startServer)
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxInfo(QObject::tr("请先启动服务器"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("请先启动服务器"),
                                 QMessageBox::Yes);
#endif
        return;
    }

    ui->pushButton_29->setEnabled(false);

    int rowCount = ui->tableWidget_3->rowCount();
    for(int i = 0; i < rowCount; i++)
    {
        ui->tableWidget_3->removeRow(0);
    }

    bool atLeastOneSelFlag = false;
    char *pchStr = new char[128];
    char *chkSum = new char;
    int currentRow = 0;
    for(int row=0; row < ui->tableWidget_2->rowCount(); row++)
    {
        if(((QCheckBox*)(ui->tableWidget_2->cellWidget(row, 0)))->isChecked())
        {
            atLeastOneSelFlag = true;
            ui->tableWidget_2->selectRow(row);
            //发送
            QString lockCode = ui->tableWidget_2->item(row, 1)->text();
            if(!(m_mapDevCodeIsKey.contains(lockCode)))
            {
#ifdef USE_SELF_MSGBOX
                myHelper::ShowMessageBoxError(QObject::tr("编号为%1的锁具未连接到服务器!").arg(lockCode));
#else
                QMessageBox::warning(NULL, QObject::tr("警告"),
                                     QObject::tr("编号为%1的锁具未连接到服务器!")
                                     .arg(lockCode),
                                     QMessageBox::Yes);
#endif
                continue;
            }

            QTcpSocket *lockSocket = m_mapDevCodeIsKey.value(lockCode);
            QString lockHexAddr;
            if(this->getLockAddress(lockSocket,lockHexAddr))
            {
#ifdef USE_SELF_MSGBOX
                myHelper::ShowMessageBoxError(QObject::tr("获取编号为%1的锁具ID失败!").arg(lockCode));
#else
                QMessageBox::warning(NULL, QObject::tr("警告"),
                                     QObject::tr("获取编号为%1的锁具ID失败!")
                                     .arg(lockCode),
                                     QMessageBox::Yes);
#endif
                continue;
            }

            if(!MysqlOperation::isLockIPTheSame(lockCode,this->hex82IP(lockHexAddr)))
            {
#ifdef USE_SELF_MSGBOX
                int ok = myHelper::ShowMessageBoxQuesion(QObject::tr("编号为%1的锁具ID与数据库中的不一致，\n是否更新？")
                                                         .arg(lockCode));
                if(ok == 1)
#else
               int ok = QMessageBox::information(NULL, QObject::tr("提示"),
                                         QObject::tr("编号为%1的锁具ID与数据库中的不一致，\n是否更新？")
                                         .arg(lockCode),
                                         QMessageBox::Yes | QMessageBox::No,
                                         QMessageBox::Yes);

               if(ok == QMessageBox::Yes)
#endif
               {
                   MysqlOperation::updateLockIP(lockCode,this->hex82IP(lockHexAddr));
               }
            }

            sprintf_s(pchStr, 128, "%sRUA,", lockHexAddr.toLocal8Bit().data());
            SPComm::getChksum(pchStr, chkSum);
            sprintf_s(pchStr, 128, "#%sRUA,%02X\r", lockHexAddr.toLocal8Bit().data(), *chkSum & 0xff);

            QByteArray readBuf;
            if(this->socketWriteAndRead(lockSocket,pchStr,readBuf))
            {
                if(readBuf.contains("??"))
                {
#ifdef USE_SELF_MSGBOX
                    myHelper::ShowMessageBoxError(QObject::tr("编号为%1的锁具，\n获取锁用户总数失败").arg(lockCode));
#else
                    QMessageBox::warning(NULL, QObject::tr("警告"),
                                         QObject::tr("编号为%1的锁具，\n获取锁用户总数失败")
                                         .arg(lockCode),
                                         QMessageBox::Yes);
#endif
                    continue;
                }
                else
                {
                    bool ok;
                    int userCount = -1;
                    if(readBuf.mid(11, 1) == ":")
                    {
                        userCount = 10;
                    }
                    else
                    {
                        userCount = readBuf.mid(11, 1).toInt(&ok, 10);
                        if(!ok)
                        {
                            userCount = 0;
                        }
                    }

                    if(userCount < 1 || userCount >10)
                    {
#ifdef USE_SELF_MSGBOX
                        myHelper::ShowMessageBoxError(QObject::tr("编号为%1的锁具，\n获取到锁用户总数为0")
                                                      .arg(lockCode));
#else
                        QMessageBox::warning(NULL, QObject::tr("警告"),
                                             QObject::tr("编号为%1的锁具，\n获取到锁用户总数为0")
                                             .arg(lockCode),
                                             QMessageBox::Yes);
#endif
                        continue;
                    }
                    else
                    {
                        //获取用户信息
                        for(int i = 0; i < userCount; i++)
                        {
                            sprintf_s(pchStr, 128, "%sRU%d,", lockHexAddr.toLocal8Bit().data(), i);
                            SPComm::getChksum(pchStr, chkSum);
                            sprintf_s(pchStr, 128, "#%sRU%d,%02X\r", lockHexAddr.toLocal8Bit().data(), i, *chkSum & 0xff);
                            QByteArray readBuf;
                            if(this->socketWriteAndRead(lockSocket,pchStr,readBuf))
                            {
                                if(!(readBuf.contains("??")))
                                {
                                    //显示锁用户数据
                                    int index = readBuf.indexOf(",");
                                    QString lockUser = readBuf.mid(11, index - 11);
                                    QString password = readBuf.mid(index + 1,
                                                                   readBuf.lastIndexOf(",")
                                                                   - index - 1);
                                    QString lockAddr,userGroup;
                                    MysqlOperation::getLockAddrGroupByCode(lockCode,lockAddr,userGroup);

                                    QCheckBox *checkBox = new QCheckBox;

                                    QTableWidgetItem *item1 = new QTableWidgetItem(lockCode);

                                    QTableWidgetItem *item2 = new QTableWidgetItem(userGroup);

                                    QTableWidgetItem *item3 = new QTableWidgetItem(lockAddr);

                                    QTableWidgetItem *item4 = new QTableWidgetItem(lockUser);

                                    QTableWidgetItem *item5 = new QTableWidgetItem(password);

                                    QTableWidgetItem *item6 = new QTableWidgetItem(QString::number(i));

                                    ui->tableWidget_3->insertRow(currentRow);
                                    ui->tableWidget_3->setCellWidget(currentRow,0,checkBox);
                                    ui->tableWidget_3->setItem(currentRow, 1, item1);
                                    ui->tableWidget_3->setItem(currentRow, 2, item2);
                                    ui->tableWidget_3->setItem(currentRow, 3, item3);
                                    ui->tableWidget_3->setItem(currentRow, 4, item4);
                                    ui->tableWidget_3->setItem(currentRow, 5, item5);
                                    ui->tableWidget_3->setItem(currentRow, 6, item6);

                                    currentRow++;

                                }
                                else
                                {
#ifdef USE_SELF_MSGBOX
                                   myHelper::ShowMessageBoxError(QObject::tr("编号为%1的锁具，\n获取锁用户%2失败")
                                                                 .arg(lockCode).arg(QString::number(i)));
#else
                                    QMessageBox::warning(NULL, QObject::tr("警告"),
                                                         QObject::tr("编号为%1的锁具，\n获取锁用户%2失败")
                                                         .arg(lockCode).arg(QString::number(i)),
                                                         QMessageBox::Yes);
#endif
                                    continue;
                                }
                            }
                        }

                        ui->tableWidget_3->insertRow(currentRow);
                        currentRow++;

                    }
                }
            }

        }//end if
    }//end for

    if(!atLeastOneSelFlag)
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxInfo(QObject::tr("请选择要查看锁用户的锁具"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("请选择要查看锁用户的锁具"),
                                 QMessageBox::Yes);
#endif
    }

    delete[] pchStr;
    pchStr = NULL;
    delete chkSum;
    chkSum = NULL;

    ui->pushButton_29->setEnabled(true);
}

void MainWindow::on_pushButton_31_clicked()
{
    ui->stackedWidget_3->setCurrentIndex(1);

    if(!m_startServer)
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxInfo(QObject::tr("请先启动服务器"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("请先启动服务器"),
                                 QMessageBox::Yes);
#endif
        return;
    }

    ui->pushButton_31->setEnabled(false);

    bool atLeastOneSelFlag = false;
    char *pchStr = new char[128];
    char *chkSum = new char;
    bool isDeleteOk = false;
    QString lockHexAddr;
    for(int row=0; row < ui->tableWidget_2->rowCount(); row++)
    {
        if(((QCheckBox*)(ui->tableWidget_2->cellWidget(row, 0)))->isChecked())
        {
            atLeastOneSelFlag = true;
            //发送
            QString lockCode = ui->tableWidget_2->item(row, 1)->text();
            if(!(m_mapDevCodeIsKey.contains(lockCode)))
            {
#ifdef USE_SELF_MSGBOX
                myHelper::ShowMessageBoxError(QObject::tr("编号为%1的锁具未连接到服务器!").arg(lockCode));
#else
                QMessageBox::warning(NULL, QObject::tr("警告"),
                                     QObject::tr("编号为%1的锁具未连接到服务器!")
                                     .arg(lockCode),
                                     QMessageBox::Yes);
#endif
                continue;
            }

            QTcpSocket *lockSocket = m_mapDevCodeIsKey.value(lockCode);

            if(this->getLockAddress(lockSocket,lockHexAddr))
            {
#ifdef USE_SELF_MSGBOX
                myHelper::ShowMessageBoxError(QObject::tr("获取编号为%1的锁具ID失败!").arg(lockCode));
#else
                QMessageBox::warning(NULL, QObject::tr("警告"),
                                     QObject::tr("获取编号为%1的锁具ID失败!")
                                     .arg(lockCode),
                                     QMessageBox::Yes);
#endif
                continue;
            }

            if(!MysqlOperation::isLockIPTheSame(lockCode,this->hex82IP(lockHexAddr)))
            {
#ifdef USE_SELF_MSGBOX
                int ok = myHelper::ShowMessageBoxQuesion(QObject::tr("编号为%1的锁具ID与数据库中的不一致，\n是否更新？")
                                                         .arg(lockCode));
                if(ok == 1)
#else
               int ok = QMessageBox::information(NULL, QObject::tr("提示"),
                                         QObject::tr("编号为%1的锁具ID与数据库中的不一致，\n是否更新？")
                                         .arg(lockCode),
                                         QMessageBox::Yes | QMessageBox::No,
                                         QMessageBox::Yes);

               if(ok == QMessageBox::Yes)
#endif
               {
                   MysqlOperation::updateLockIP(lockCode,this->hex82IP(lockHexAddr));
               }
            }

            QString numID = ui->tableWidget_3->item(row, 6)->text();
            sprintf_s(pchStr, 128, "%sWD%d,", lockHexAddr.toLocal8Bit().data(), numID.toInt());
            SPComm::getChksum(pchStr, chkSum);
            sprintf_s(pchStr, 128, "#%sWD%d,%02X\r", lockHexAddr.toLocal8Bit().data(),
                      numID.toInt(), *chkSum & 0xff);

            //发送与接收
            QByteArray readBuf;
            if(this->socketWriteAndRead(lockSocket,pchStr,readBuf))
            {
                if(readBuf.contains("OK"))
                {
                    isDeleteOk = true;
                    ui->tableWidget_3->removeRow(row);
                    continue;
                }
                else
                {
                    isDeleteOk = false;
#ifdef USE_SELF_MSGBOX
                    myHelper::ShowMessageBoxError(QObject::tr("编号为%1的锁具，\n删除锁用户%2失败")
                                                  .arg(lockCode).arg(numID));
#else
                    QMessageBox::warning(NULL, QObject::tr("警告"),
                                         QObject::tr("编号为%1的锁具，\n删除锁用户%2失败")
                                         .arg(lockCode).arg(numID),
                                         QMessageBox::Yes);
#endif
                }
            }
        }
    }

    if(!atLeastOneSelFlag)
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxInfo(QObject::tr("请选择要删除锁用户的锁具"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("请选择要删除锁用户的锁具"),
                                 QMessageBox::Yes);
#endif
    }
    else
    {
        if(isDeleteOk)
        {
            //记录日志
            if(ui->comboBox->currentText() == tr("管理员"))
            {
                MysqlOperation::insertRecord(ui->lineEdit->text(),
                                            "",
                                             "NNNN4N6N",tr("删除锁用户成功"));
            }
            else
            {
                MysqlOperation::insertRecord(ui->lineEdit->text(),
                                             "",
                                             "NNNN4NN7",tr("删除锁用户成功"));
            }
        }
    }

    delete[] pchStr;
    pchStr = NULL;
    delete chkSum;
    chkSum = NULL;

    ui->pushButton_31->setEnabled(true);
}

void MainWindow::on_pushButton_32_clicked()
{
    ui->stackedWidget_3->setCurrentIndex(2);
}

void MainWindow::on_pushButton_35_clicked()
{
    ui->stackedWidget_3->setCurrentIndex(3);

    if(!m_startServer)
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxInfo(QObject::tr("请先启动服务器"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("请先启动服务器"),
                                 QMessageBox::Yes);
#endif
        return;
    }

    ui->pushButton_35->setEnabled(false);

    int rowCount = ui->tableWidget_5->rowCount();
    for(int i = 0; i < rowCount; i++)
    {
        ui->tableWidget_5->removeRow(0);
    }

    bool atLeastOneSelFlag = false;
    char *pchStr = new char[128];
    char *chkSum = new char;
    int currentRow = 0;
    for(int row=0; row < ui->tableWidget_2->rowCount(); row++)
    {
        if(((QCheckBox*)(ui->tableWidget_2->cellWidget(row, 0)))->isChecked())
        {
            atLeastOneSelFlag = true;
            ui->tableWidget_2->selectRow(row);
            //发送
            QString lockCode = ui->tableWidget_2->item(row, 1)->text();
            if(!(m_mapDevCodeIsKey.contains(lockCode)))
            {
#ifdef USE_SELF_MSGBOX
                myHelper::ShowMessageBoxError(QObject::tr("编号为%1的锁具未连接到服务器!").arg(lockCode));
#else
                QMessageBox::warning(NULL, QObject::tr("警告"),
                                     QObject::tr("编号为%1的锁具未连接到服务器!")
                                     .arg(lockCode),
                                     QMessageBox::Yes);
#endif
                continue;
            }

            QTcpSocket *lockSocket = m_mapDevCodeIsKey.value(lockCode);
            QString lockHexAddr;
            if(this->getLockAddress(lockSocket,lockHexAddr))
            {
#ifdef USE_SELF_MSGBOX
                myHelper::ShowMessageBoxError(QObject::tr("获取编号为%1的锁具ID失败!").arg(lockCode));
#else
                QMessageBox::warning(NULL, QObject::tr("警告"),
                                     QObject::tr("获取编号为%1的锁具ID失败!")
                                     .arg(lockCode),
                                     QMessageBox::Yes);
#endif
                continue;
            }

            if(!MysqlOperation::isLockIPTheSame(lockCode,this->hex82IP(lockHexAddr)))
            {
#ifdef USE_SELF_MSGBOX
                int ok = myHelper::ShowMessageBoxQuesion(QObject::tr("编号为%1的锁具ID与数据库中的不一致，\n是否更新？")
                                                         .arg(lockCode));
                if(ok == 1)
#else
               int ok = QMessageBox::information(NULL, QObject::tr("提示"),
                                         QObject::tr("编号为%1的锁具ID与数据库中的不一致，\n是否更新？")
                                         .arg(lockCode),
                                         QMessageBox::Yes | QMessageBox::No,
                                         QMessageBox::Yes);

               if(ok == QMessageBox::Yes)
#endif
               {
                   MysqlOperation::updateLockIP(lockCode,this->hex82IP(lockHexAddr));
               }
            }

            sprintf_s(pchStr, 128, "%sRLA,", lockHexAddr.toLocal8Bit().data());
            SPComm::getChksum(pchStr, chkSum);
            sprintf_s(pchStr, 128, "#%sRLA,%02X\r", lockHexAddr.toLocal8Bit().data(), *chkSum & 0xff);

            QByteArray readBuf;
            if(this->socketWriteAndRead(lockSocket,pchStr,readBuf))
            {
                if(readBuf.contains("??"))
                {
#ifdef USE_SELF_MSGBOX
                    myHelper::ShowMessageBoxError(QObject::tr("编号为%1的锁具，\n获取锁日志总数失败")
                                                  .arg(lockCode));
#else
                    QMessageBox::warning(NULL, QObject::tr("警告"),
                                         QObject::tr("编号为%1的锁具，\n获取锁日志总数失败")
                                         .arg(lockCode),
                                         QMessageBox::Yes);
#endif
                    continue;
                }
                else
                {
                    bool ok;
                    int openCount = -1;

                    openCount = readBuf.mid(12, readBuf.indexOf(",") - 12).toInt(&ok, 10);
                    if(!ok)
                    {
                        openCount = 0;
                    }
                    if(openCount < 1 || openCount > 500)
                    {
#ifdef USE_SELF_MSGBOX
                        myHelper::ShowMessageBoxInfo(QObject::tr("获取到开锁记录总数为0"));
#else
                        QMessageBox::information(NULL, QObject::tr("提示"),
                                                 QObject::tr("获取到开锁记录总数为0"),
                                                 QMessageBox::Yes);
#endif
                    }
                    else
                    {
                        int count = -1;
                        if(openCount < 20)
                        {
                            count = openCount;
                        }
                        else
                        {
                            count = 20;
                        }
                        //获取开锁日志
                        for(int i = 0; i < count; i++)
                        {
                            sprintf_s(pchStr, 128, "%sRL%03d,", lockHexAddr.toLocal8Bit().data(), i);
                            SPComm::getChksum(pchStr, chkSum);
                            sprintf_s(pchStr, 128, "#%sRL%03d,%X\r", lockHexAddr.toLocal8Bit().data(), i, *chkSum & 0xff);

                            QByteArray readBuf;
                            if(this->socketWriteAndRead(lockSocket,pchStr,readBuf))
                            {
                                if(!(readBuf.contains("??")))
                                {
                                    //显示开锁记录
                                    int index = readBuf.indexOf(",");
                                    QString recordTime = readBuf.mid(11, index - 11);
                                    QString recordName = readBuf.mid(index + 1,
                                                                             readBuf.lastIndexOf(",") - index - 1);

                                    QString lockName;
                                    QString lockAddr;
                                    QString lockGroup;
                                    MysqlOperation::getLockNameAddrGroupByCode(lockCode,lockName,lockAddr,lockGroup);

                                    QVector<QString> adminInfoVector;
                                    MysqlOperation::getAdminInfoByLogName(recordName,adminInfoVector);

                                    QTableWidgetItem *item0 = new QTableWidgetItem(lockCode);
                                    QTableWidgetItem *item1 = new QTableWidgetItem(lockGroup);

                                    QTableWidgetItem *item2 = new QTableWidgetItem(
                                                "于" + recordTime.left(4) +
                                                "-" + recordTime.mid(4, 2) +
                                                "-" + recordTime.mid(6, 2) +
                                                ", " + recordTime.mid(8, 2) +
                                                ":" + recordTime.mid(10, 2) +
                                                ":" + recordTime.mid(12, 2) + "开锁成功");
                                    QTableWidgetItem *item3 = new QTableWidgetItem(adminInfoVector.at(0));
                                    QTableWidgetItem *item4 = new QTableWidgetItem(recordName);
                                    QTableWidgetItem *item5 = new QTableWidgetItem(adminInfoVector.at(4));
                                    QTableWidgetItem *item6 = new QTableWidgetItem(adminInfoVector.at(5));


                                    ui->tableWidget_5->insertRow(currentRow);
                                    ui->tableWidget_5->setItem(currentRow, 0, item0);
                                    ui->tableWidget_5->setItem(currentRow, 1, item1);
                                    ui->tableWidget_5->setItem(currentRow, 2, item2);
                                    ui->tableWidget_5->setItem(currentRow, 3, item3);
                                    ui->tableWidget_5->setItem(currentRow, 4, item4);
                                    ui->tableWidget_5->setItem(currentRow, 5, item5);
                                    ui->tableWidget_5->setItem(currentRow, 6, item6);

                                    currentRow++;

                                    //记录日志
                                    if(adminInfoVector.at(3) == tr("管理员"))
                                    {
                                        MysqlOperation::insertRecord(adminInfoVector.at(0),
                                                                    lockCode,
                                                                     "NNNN4N6N",tr("开锁成功，时间:%1")
                                                                     .arg(recordTime));
                                    }
                                    else
                                    {
                                        MysqlOperation::insertRecord(ui->lineEdit->text(),
                                                                     lockCode,
                                                                     "NNNN4NN7",tr("监控中心开锁成功")
                                                                     .arg(recordTime));
                                    }

                                }
                                else
                                {
#ifdef USE_SELF_MSGBOX
                                    myHelper::ShowMessageBoxError(QObject::tr("编号为%1的锁具，\n获取开锁记录%2失败")
                                                                  .arg(lockCode).arg(QString::number(i)));
#else
                                    QMessageBox::warning(NULL, QObject::tr("警告"),
                                                         QObject::tr("编号为%1的锁具，\n获取开锁记录%2失败")
                                                         .arg(lockCode).arg(QString::number(i)),
                                                         QMessageBox::Yes);
#endif
                                    continue;
                                }
                            }
                        }

                        ui->tableWidget_5->insertRow(currentRow);
                        currentRow++;

                    }
                }
            }

        }//end if
    }//end for

    if(!atLeastOneSelFlag)
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxInfo(QObject::tr("请选择要查看开锁记录的锁具"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("请选择要查看开锁记录的锁具"),
                                 QMessageBox::Yes);
#endif
    }

    delete[] pchStr;
    pchStr = NULL;
    delete chkSum;
    chkSum = NULL;

    ui->pushButton_35->setEnabled(true);
}

void MainWindow::on_pushButton_69_clicked()
{
    this->updateGroupTableView();
    this->updateGroupComboBox();
}

void MainWindow::on_pushButton_33_clicked()
{
    int rowNum = m_groupModel.rowCount();
    m_groupModel.insertRow(rowNum); //添加一行

    QDateTime currentDateTime = QDateTime::currentDateTime();
    m_groupModel.setData(m_groupModel.index(rowNum, 2), currentDateTime);
}

void MainWindow::on_pushButton_95_clicked()
{
    m_groupModel.database().transaction(); //开始事务操作
    if (m_groupModel.submitAll())
    {
        m_groupModel.database().commit(); //提交
    }
    else
    {
        m_groupModel.database().rollback(); //回滚
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("数据库错误: %1")
                                      .arg(m_groupModel.lastError().text()));
#else
        QMessageBox::warning(NULL, QObject::tr("警告"),
                             QObject::tr("数据库错误: %1")
                             .arg(m_groupModel.lastError().text()),
                             QMessageBox::Yes);
#endif
    }
}

void MainWindow::on_pushButton_38_clicked()
{
    int curRow = ui->tableView_6->currentIndex().row();  //获取选中的行
    if(curRow == -1)
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxInfo(QObject::tr("请选择要删除的用户组"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("请选择要删除的用户组"),
                                 QMessageBox::Yes);
#endif
        return;
    }

#ifdef USE_SELF_MSGBOX
    int ok = myHelper::ShowMessageBoxQuesion(QObject::tr("是否删除第%1行的用户组？")
                                             .arg(QString::number(curRow + 1, 10)));
    if(ok == 1)
#else
    int ok = QMessageBox::warning(NULL, QObject::tr("警告"),
                                  QObject::tr("是否删除第%1行的用户组？")
                                  .arg(QString::number(curRow + 1, 10)),
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::Yes);
    if(ok == QMessageBox::Yes)
#endif
    {
        m_groupModel.removeRow(curRow);  //删除该行
        m_groupModel.submitAll(); //在数据库中删除该行
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxInfo(QObject::tr("删除成功"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("删除成功"),
                                 QMessageBox::Yes);
#endif

    }
}

void MainWindow::on_pushButton_96_clicked()
{
    m_groupModel.revertAll();
}

void MainWindow::on_pushButton_10_clicked()
{
    this->updateAdminTableView();
}

void MainWindow::on_pushButton_9_clicked()
{
    m_adminModel.database().transaction(); //开始事务操作
    if (m_adminModel.submitAll())
    {
        m_adminModel.database().commit(); //提交
    }
    else
    {
        m_adminModel.database().rollback(); //回滚
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("数据库错误: %1")
                                      .arg(m_adminModel.lastError().text()));
#else
        QMessageBox::warning(NULL, QObject::tr("警告"),
                             QObject::tr("数据库错误: %1")
                             .arg(m_adminModel.lastError().text()),
                             QMessageBox::Yes);
#endif
    }

    this->updateAdminTableView();
}

void MainWindow::on_pushButton_7_clicked()
{
    int curRow = ui->tableView_2->currentIndex().row();  //获取选中的行
    if(curRow == -1)
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxInfo(QObject::tr("请选择要删除的用户"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("请选择要删除的用户"),
                                 QMessageBox::Yes);
#endif
        return;
    }
#ifdef USE_SELF_MSGBOX
    int ok = myHelper::ShowMessageBoxQuesion(QObject::tr("是否删除第%1行的用户信息？")
                                             .arg(QString::number(curRow + 1, 10)));
    if(ok == 1)
#else
    int ok = QMessageBox::warning(NULL, QObject::tr("警告"),
                                  QObject::tr("是否删除第%1行的用户信息？")
                                  .arg(QString::number(curRow + 1, 10)),
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::Yes);
    if(ok == QMessageBox::Yes)
#endif
    {
        m_adminModel.removeRow(curRow);  //删除该行
        m_adminModel.submitAll(); //在数据库中删除该行
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxInfo(QObject::tr("删除成功"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("删除成功"),
                                 QMessageBox::Yes);
#endif

    }
}

void MainWindow::on_pushButton_11_clicked()
{
    m_adminModel.revertAll();
}

void MainWindow::on_pushButton_20_clicked()
{
    m_adminModel.setTable("adminTable");
    m_adminModel.setEditStrategy(QSqlTableModel::OnManualSubmit);
    if(ui->radioButton_6->isChecked())
    {
        QString name = ui->lineEdit_6->text().trimmed();
        m_adminModel.setFilter(QString("callName = '%1'").arg(name));
    }
    if(ui->radioButton_7->isChecked())
    {
        QString powerStatus = ui->comboBox_5->currentText();
        m_adminModel.setFilter(tr("empower = '%1'").arg(powerStatus));
    }
    if(ui->radioButton_12->isChecked())
    {
        QString groupName = ui->comboBox_8->currentText();
        m_adminModel.setFilter(tr("userGroup = '%1'").arg(groupName));
    }

    m_adminModel.select();
    m_adminModel.setHeaderData(1, Qt::Horizontal, QObject::tr("姓名"));
    m_adminModel.setHeaderData(2, Qt::Horizontal, QObject::tr("登录名"));
    m_adminModel.setHeaderData(3, Qt::Horizontal, QObject::tr("密码"));
    m_adminModel.setHeaderData(4, Qt::Horizontal, QObject::tr("权限组"));
    m_adminModel.setHeaderData(5, Qt::Horizontal, QObject::tr("用户组"));
    m_adminModel.setHeaderData(6, Qt::Horizontal, QObject::tr("联系方式"));
    m_adminModel.setHeaderData(7, Qt::Horizontal, QObject::tr("权限状态"));
    m_adminModel.setHeaderData(8, Qt::Horizontal, QObject::tr("权限起始时间"));
    m_adminModel.setHeaderData(9, Qt::Horizontal, QObject::tr("权限截止时间"));

    QVector<QString> vector4;
    vector4.append(tr("操作员"));
    vector4.append(tr("管理员"));
    QComboBoxDelegate *comboBoxDelegate4 = new QComboBoxDelegate(vector4);
    ui->tableView_2->setItemDelegateForColumn(4, comboBoxDelegate4);

    QVector<QString> vector5;
    MysqlOperation::getGroupInfo(vector5);
    QComboBoxDelegate *comboBoxDelegate5 = new QComboBoxDelegate(vector5);
    ui->tableView_2->setItemDelegateForColumn(5, comboBoxDelegate5);

    QVector<QString> vector7;
    vector7.append(tr("未授权"));
    vector7.append(tr("期间授权"));
    vector7.append(tr("永久授权"));
    vector7.append(tr("权限过期"));
    vector7.append(tr("权限未到"));
    vector7.append(tr("列入黑名单"));
    QComboBoxDelegate *comboBoxDelegate7 = new QComboBoxDelegate(vector7);
    ui->tableView_2->setItemDelegateForColumn(7, comboBoxDelegate7);
    ui->tableView_2->setModel(&m_adminModel);
    ui->tableView_2->hideColumn(0);
    ui->tableView_2->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

void MainWindow::on_pushButton_22_clicked()
{
    this->updateLockTableView();
}

void MainWindow::on_pushButton_17_clicked()
{
    m_lockModel.database().transaction(); //开始事务操作
    if (m_lockModel.submitAll())
    {
        m_lockModel.database().commit(); //提交
    }
    else
    {
        m_lockModel.database().rollback(); //回滚
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("数据库错误: %1")
                                      .arg(m_lockModel.lastError().text()));
#else
        QMessageBox::warning(NULL, QObject::tr("警告"),
                             QObject::tr("数据库错误: %1")
                             .arg(m_lockModel.lastError().text()),
                             QMessageBox::Yes);
#endif
    }

    this->updateLockTableView();
}

void MainWindow::on_pushButton_24_clicked()
{
    int rowNum = m_lockModel.rowCount();
    m_lockModel.insertRow(rowNum); //添加一行

    QDateTime currentDateTime = QDateTime::currentDateTime();
    m_lockModel.setData(m_lockModel.index(rowNum, 9), currentDateTime);
}

void MainWindow::on_pushButton_23_clicked()
{
    int curRow = ui->tableView_5->currentIndex().row();  //获取选中的行
    if(curRow == -1)
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxInfo(QObject::tr("请选择要删除的锁"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("请选择要删除的锁"),
                                 QMessageBox::Yes);
#endif
        return;
    }
#ifdef USE_SELF_MSGBOX
    int ok = myHelper::ShowMessageBoxQuesion(QObject::tr("是否删除第%1行的锁信息？")
                                             .arg(QString::number(curRow + 1, 10)));
    if(ok == 1)
#else
    int ok = QMessageBox::warning(NULL, QObject::tr("警告"),
                                  QObject::tr("是否删除第%1行的锁信息？")
                                  .arg(QString::number(curRow + 1, 10)),
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::Yes);
    if(ok == QMessageBox::Yes)
#endif
    {
        m_lockModel.removeRow(curRow);  //删除该行
        m_lockModel.submitAll(); //在数据库中删除该行
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxInfo(QObject::tr("删除成功"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("删除成功"),
                                 QMessageBox::Yes);
#endif

    }
}

void MainWindow::on_pushButton_28_clicked()
{
    m_lockModel.revertAll();
}

void MainWindow::on_pushButton_18_clicked()
{
    m_lockModel.setTable("lockTable");
    m_lockModel.setEditStrategy(QSqlTableModel::OnManualSubmit);
    if(ui->radioButton_8->isChecked())
    {
        QString lockCode = ui->lineEdit_10->text().trimmed();
        m_lockModel.setFilter(QString("lockCode = '%1'").arg(lockCode));
    }
    if(ui->radioButton_9->isChecked())
    {
        QString groupName = ui->comboBox_6->currentText();
        m_lockModel.setFilter(tr("userGroup = '%1'").arg(groupName));
    }

    m_lockModel.select();
    m_lockModel.setHeaderData(1, Qt::Horizontal, QObject::tr("锁编号"));
    m_lockModel.setHeaderData(2, Qt::Horizontal, QObject::tr("锁名称"));
    m_lockModel.setHeaderData(3, Qt::Horizontal, QObject::tr("所属用户组"));
    m_lockModel.setHeaderData(4, Qt::Horizontal, QObject::tr("地理位置"));
    m_lockModel.setHeaderData(5, Qt::Horizontal, QObject::tr("经度"));
    m_lockModel.setHeaderData(6, Qt::Horizontal, QObject::tr("纬度"));
    m_lockModel.setHeaderData(7, Qt::Horizontal, QObject::tr("ip地址"));
    int ret = MysqlOperation::isAdminRoot(ui->lineEdit_2->text().trimmed(),ui->lineEdit_3->text().trimmed());
    if(ret == 1)
    {
        m_lockModel.setHeaderData(8, Qt::Horizontal, QObject::tr("锁用户"));
    }
    m_lockModel.setHeaderData(9, Qt::Horizontal, QObject::tr("服役时间"));
    if(ret == 1)
    {
        m_lockModel.setHeaderData(10, Qt::Horizontal, QObject::tr("锁ID"));
    }
    QVector<QString> vector3;
    MysqlOperation::getGroupInfo(vector3);
    QComboBoxDelegate *comboBoxDelegate3 = new QComboBoxDelegate(vector3);
    ui->tableView_5->setItemDelegateForColumn(3, comboBoxDelegate3);


    ui->tableView_5->setModel(&m_lockModel);
    ui->tableView_5->hideColumn(0);
    if(ret != 1)
    {
        ui->tableView_5->hideColumn(8);
        ui->tableView_5->hideColumn(10);
    }

    ui->tableView_5->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

void MainWindow::on_pushButton_44_clicked()
{
    this->updateKeyTableView();
}

void MainWindow::on_pushButton_41_clicked()
{
    int curRow = ui->tableView_7->currentIndex().row();  //获取选中的行
    if(curRow == -1)
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxInfo(QObject::tr("请选择要删除的钥匙"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("请选择要删除的钥匙"),
                                 QMessageBox::Yes);
#endif
        return;
    }
#ifdef USE_SELF_MSGBOX
    int ok = myHelper::ShowMessageBoxQuesion(QObject::tr("是否删除第%1行的钥匙信息？")
                                             .arg(QString::number(curRow + 1, 10)));
    if(ok == 1)
#else
    int ok = QMessageBox::warning(NULL, QObject::tr("警告"),
                                  QObject::tr("是否删除第%1行的钥匙信息？")
                                  .arg(QString::number(curRow + 1, 10)),
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::Yes);
    if(ok == QMessageBox::Yes)
#endif
    {
        m_keyModel.removeRow(curRow);  //删除该行
        m_keyModel.submitAll(); //在数据库中删除该行
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxInfo(QObject::tr("删除成功"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("删除成功"),
                                 QMessageBox::Yes);
#endif

    }
}

void MainWindow::on_pushButton_40_clicked()
{
    m_keyModel.setTable("keyTable");
    m_keyModel.setEditStrategy(QSqlTableModel::OnManualSubmit);
    if(ui->radioButton_10->isChecked())
    {
        QString keyCode = ui->lineEdit_11->text().trimmed();
        m_lockModel.setFilter(QString("keyCode = '%1'").arg(keyCode));
    }
    if(ui->radioButton_11->isChecked())
    {
        QString groupName = ui->comboBox_7->currentText();
        m_keyModel.setFilter(tr("userGroup = '%1'").arg(groupName));
    }
    m_keyModel.select();
    m_keyModel.setHeaderData(1, Qt::Horizontal, QObject::tr("钥匙编号"));
    m_keyModel.setHeaderData(2, Qt::Horizontal, QObject::tr("钥匙名称"));
    m_keyModel.setHeaderData(3, Qt::Horizontal, QObject::tr("所属用户组"));
    m_keyModel.setHeaderData(4, Qt::Horizontal, QObject::tr("A级密钥"));
    m_keyModel.setHeaderData(5, Qt::Horizontal, QObject::tr("B级密钥"));
    m_keyModel.setHeaderData(6, Qt::Horizontal, QObject::tr("权限状态"));
    m_keyModel.setHeaderData(7, Qt::Horizontal, QObject::tr("权限掩码"));
    m_keyModel.setHeaderData(8, Qt::Horizontal, QObject::tr("权限起始日期"));
    m_keyModel.setHeaderData(9, Qt::Horizontal, QObject::tr("权限截止日期"));
    int ret = MysqlOperation::isAdminRoot(ui->lineEdit_2->text().trimmed(),ui->lineEdit_3->text().trimmed());
    if(ret == 1)
    {
        m_keyModel.setHeaderData(10, Qt::Horizontal, QObject::tr("钥匙ID"));
    }
    QVector<QString> vector6;
    vector6.append(tr("1开254"));
    vector6.append(tr("1开254^2"));
    vector6.append(tr("1开254^3"));
    vector6.append(tr("全开"));
    vector6.append(tr("权限过期"));
    vector6.append(tr("权限未到"));
    QComboBoxDelegate *comboBoxDelegate6 = new QComboBoxDelegate(vector6);
    ui->tableView_7->setItemDelegateForColumn(6, comboBoxDelegate6);

    ui->tableView_7->setModel(&m_keyModel);
    ui->tableView_7->hideColumn(0);
    if(ret != 1)
    {
        ui->tableView_7->hideColumn(10);
    }

    ui->tableView_7->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

void MainWindow::on_pushButton_4_clicked()
{
    this->updateEmpowerTableView();
}

void MainWindow::on_pushButton_8_clicked()
{
    m_empowerModel.database().transaction(); //开始事务操作
    if (m_empowerModel.submitAll())
    {
        m_empowerModel.database().commit(); //提交
    }
    else
    {
        m_empowerModel.database().rollback(); //回滚
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("数据库错误: %1")
                                      .arg(m_empowerModel.lastError().text()));
#else
        QMessageBox::warning(NULL, QObject::tr("警告"),
                             QObject::tr("数据库错误: %1")
                             .arg(m_empowerModel.lastError().text()),
                             QMessageBox::Yes);
#endif
    }

    this->updateEmpowerTableView();
}

void MainWindow::on_pushButton_26_clicked()
{
    m_empowerModel.revertAll();
}

void MainWindow::on_pushButton_13_clicked()
{
    m_empowerModel.setTable("adminTable");
    m_empowerModel.setEditStrategy(QSqlTableModel::OnManualSubmit);
    if(ui->radioButton_3->isChecked())
    {
        QString name = ui->lineEdit_9->text().trimmed();
        m_empowerModel.setFilter(QString("callName = '%1'").arg(name));
    }
    if(ui->radioButton_4->isChecked())
    {
        QString powerStatus = ui->comboBox_3->currentText();
        m_empowerModel.setFilter(tr("empower = '%1'").arg(powerStatus));
    }

    m_empowerModel.select();
    m_empowerModel.setHeaderData(1, Qt::Horizontal, QObject::tr("姓名"));
    m_empowerModel.setHeaderData(2, Qt::Horizontal, QObject::tr("登录名"));
    m_empowerModel.setHeaderData(4, Qt::Horizontal, QObject::tr("权限组"));
    m_empowerModel.setHeaderData(5, Qt::Horizontal, QObject::tr("用户组"));
    m_empowerModel.setHeaderData(6, Qt::Horizontal, QObject::tr("联系方式"));
    m_empowerModel.setHeaderData(7, Qt::Horizontal, QObject::tr("权限状态"));
    m_empowerModel.setHeaderData(8, Qt::Horizontal, QObject::tr("权限起始时间"));
    m_empowerModel.setHeaderData(9, Qt::Horizontal, QObject::tr("权限截止时间"));

    QVector<QString> vector4;
    vector4.append(tr("操作员"));
    vector4.append(tr("管理员"));
    QComboBoxDelegate *comboBoxDelegate4 = new QComboBoxDelegate(vector4);
    ui->tableView->setItemDelegateForColumn(4, comboBoxDelegate4);

    QVector<QString> vector5;
    MysqlOperation::getGroupInfo(vector5);
    QComboBoxDelegate *comboBoxDelegate5 = new QComboBoxDelegate(vector5);
    ui->tableView->setItemDelegateForColumn(5, comboBoxDelegate5);

    QVector<QString> vector7;
    vector7.append(tr("未授权"));
    vector7.append(tr("期间授权"));
    vector7.append(tr("永久授权"));
    vector7.append(tr("权限过期"));
    vector7.append(tr("权限未到"));
    vector7.append(tr("列入黑名单"));
    QComboBoxDelegate *comboBoxDelegate7 = new QComboBoxDelegate(vector7);
    ui->tableView->setItemDelegateForColumn(7, comboBoxDelegate7);
    ui->tableView->setModel(&m_empowerModel);
    ui->tableView->hideColumn(0);
    ui->tableView->hideColumn(3);
    ui->tableView->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

void MainWindow::on_pushButton_19_clicked()
{
    this->updateLoadEmpowerTableView();
}

void MainWindow::on_pushButton_5_clicked()
{
    QItemSelectionModel *selections = ui->tableView_4->selectionModel();
    QModelIndexList selected = selections->selectedIndexes();

    bool flag = false;
    foreach (QModelIndex index, selected)
    {
        if(!flag)
        {
#ifdef USE_SELF_MSGBOX
            int ok = myHelper::ShowMessageBoxQuesion(QObject::tr("是否删除选中的记录？"));
            if(ok != 1)
#else
            int ok = QMessageBox::warning(NULL, QObject::tr("警告"),
                                          QObject::tr("是否删除选中的记录？"),
                                          QMessageBox::Yes | QMessageBox::No,
                                          QMessageBox::Yes);
            if(ok == QMessageBox::No)
#endif
            {
                return;
            }
        }

        flag = true;
        m_loadEmpowerModel.removeRow(index.row());  //删除该行
    }

    if(!flag)
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxInfo(QObject::tr("请选择要删除的记录"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("请选择要删除的记录"),
                                 QMessageBox::Yes);
#endif
        return;
    }

    if(m_loadEmpowerModel.submitAll())
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxInfo(QObject::tr("删除成功"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("删除成功"),
                                 QMessageBox::Yes);
#endif

        this->updateLoadEmpowerTableView();
    }
    else
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("删除失败"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("删除失败"),
                                 QMessageBox::Yes);
#endif
    }
}

void MainWindow::on_pushButton_3_clicked()
{
    m_loadEmpowerModel.setTable("empowerTable");
    m_loadEmpowerModel.setEditStrategy(QSqlTableModel::OnManualSubmit);
    if(ui->radioButton->isChecked())
    {
        QString name = ui->lineEdit_8->text().trimmed();
        m_loadEmpowerModel.setFilter(QString("callName = '%1'").arg(name));
    }
    if(ui->radioButton_5->isChecked())
    {
        QString powerStatus = ui->comboBox_4->currentText();
        m_loadEmpowerModel.setFilter(tr("empower = '%1'").arg(powerStatus));
    }
    if(ui->radioButton_2->isChecked())
    {
        QDateTime time = ui->dateTimeEdit_3->dateTime();
        qDebug() << tr("按时间查询:") << time.toString("yyyy-MM-dd hh:mm:ss");
        m_loadEmpowerModel.setFilter(tr("loadTime >= '%1'")
                                     .arg(time.toString("yyyy-MM-dd hh:mm:ss")));
    }
    m_loadEmpowerModel.select();
    m_loadEmpowerModel.setHeaderData(1, Qt::Horizontal, QObject::tr("姓名"));
    m_loadEmpowerModel.setHeaderData(2, Qt::Horizontal, QObject::tr("登录名"));
    m_loadEmpowerModel.setHeaderData(3, Qt::Horizontal, QObject::tr("权限组"));
    m_loadEmpowerModel.setHeaderData(4, Qt::Horizontal, QObject::tr("用户组"));
    m_loadEmpowerModel.setHeaderData(5, Qt::Horizontal, QObject::tr("联系方式"));
    m_loadEmpowerModel.setHeaderData(6, Qt::Horizontal, QObject::tr("权限状态"));
    m_loadEmpowerModel.setHeaderData(7, Qt::Horizontal, QObject::tr("权限起始时间"));
    m_loadEmpowerModel.setHeaderData(8, Qt::Horizontal, QObject::tr("权限截止时间"));
    m_loadEmpowerModel.setHeaderData(9, Qt::Horizontal, QObject::tr("下载票据时间"));

    ui->tableView_4->setModel(&m_loadEmpowerModel);
    ui->tableView_4->hideColumn(0);
    ui->tableView_4->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

void MainWindow::on_pushButton_70_clicked()
{
    SPComm::setPortName(ui->comboBox_9->currentText());
    SPComm::setBaudRate(ui->comboBox_10->currentText().toInt(NULL,10));

    if(SPComm::m_serialPort->isOpen())
    {
        SPComm::close();
        ui->pushButton_70->setText(tr("打开串口"));
        ui->pushButton_70->setIcon(QIcon(":/image/ball_off.ico"));
    }
    else
    {
        if(!SPComm::open())
        {
#ifdef USE_SELF_MSGBOX
            myHelper::ShowMessageBoxError(QObject::tr("打开串口错误: %1")
                                          .arg(SPComm::m_serialPort->error()));
#else
            QMessageBox::warning(NULL, QObject::tr("警告"),
                                 QObject::tr("打开串口错误: %1")
                                 .arg(SPComm::m_serialPort->error()),
                                 QMessageBox::Yes);
#endif
            return;
        }
        ui->pushButton_70->setText(tr("关闭串口"));
        ui->pushButton_70->setIcon(QIcon(":/image/ball_on.ico"));
    }
}

void MainWindow::on_pushButton_12_clicked()
{
    this->updatePortNameComboBox();
}

void MainWindow::on_pushButton_57_clicked()
{
    QString passwordA = ui->lineEdit_11->text().trimmed();
    if(passwordA.isEmpty()||passwordA.isNull())
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("密钥不能为空"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("密钥不能为空"),
                                 QMessageBox::Yes);
#endif
        return;
    }

    if(passwordA.length() > 6)
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("密钥不能大于6位"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("密钥不能大于6位"),
                                 QMessageBox::Yes);
#endif
        return;
    }

    QString KEY_ADDRESS;
    if(!this->getKeyAddress(KEY_ADDRESS))
    {
        return;
    }

    char *pchStr = new char[128];
    char *chkSum = new char;
    sprintf_s(pchStr, 128, "%sG%s,", KEY_ADDRESS.toLocal8Bit().data(), passwordA.toLocal8Bit().data());
    SPComm::getChksum(pchStr, chkSum);
    sprintf_s(pchStr, 128, "#%sG%s,%02X\r\n", KEY_ADDRESS.toLocal8Bit().data(),
              passwordA.toLocal8Bit().data(), *chkSum & 0xff);
    //写串口
    qDebug() << "串口发送:" << pchStr;

#ifndef SHOW_COM_DATA
    ui->textBrowser->append(tr("发送:获取A级权限指令"));
#else
    ui->textBrowser->append(tr("发送:") + pchStr);
#endif

    int retWrite = SPComm::writeData(pchStr, strlen(pchStr));
    if(retWrite < 0)
    {
        delete[] pchStr;
        pchStr = NULL;
        delete chkSum;
        chkSum = NULL;

        return;
    }

    //读串口
    bool retRead = SPComm::readAllData(pchStr, 1500);
    if(retRead)
    {
        qDebug() << "串口接收:" <<pchStr;

        QString keyRet = pchStr;

        if(keyRet.contains("OK"))
        {
#ifndef SHOW_COM_DATA
            ui->textBrowser->append(tr("接收:获取A级权限成功"));
#else
            ui->textBrowser->append(tr("接收:") + pchStr);
#endif

#ifdef USE_SELF_MSGBOX
            myHelper::ShowMessageBoxInfo(QObject::tr("获取A级权限成功"));
#else
            QMessageBox::information(NULL, QObject::tr("提示"),
                                     QObject::tr("获取A级权限成功"),
                                     QMessageBox::Yes);
#endif

            ui->pushButton_16->setEnabled(false);//恢复出厂设置
            ui->pushButton_47->setEnabled(false);//重置A级密钥
            ui->pushButton_48->setEnabled(false);//重置B级密钥
            ui->pushButton_50->setEnabled(true);//制作钥匙
            ui->pushButton_55->setEnabled(false);//查看或重置钥匙编码
            ui->pushButton_56->setEnabled(true);//查看日志

            //数据库操作
            MysqlOperation::updateSKA(this->hex82IP(KEY_ADDRESS),passwordA);
            this->updateKeyTableView();

            //记录日志
            if(ui->comboBox->currentText() == tr("管理员"))
            {
                MysqlOperation::insertRecord(ui->lineEdit->text(),
                                             MysqlOperation::getkeyCodeBykeyIP(hex82IP(KEY_ADDRESS)),
                                             "NNN3NN6N","获取A级密钥权限成功");
            }
            else
            {
                MysqlOperation::insertRecord(ui->lineEdit->text(),
                                             MysqlOperation::getkeyCodeBykeyIP(hex82IP(KEY_ADDRESS)),
                                             "NNN3NNN7","获取A级密钥权限成功");
            }

            delete[] pchStr;
            pchStr = NULL;
            delete chkSum;
            chkSum = NULL;

            return;
        }
    }

#ifndef SHOW_COM_DATA
    ui->textBrowser->append(tr("接收:获取A级权限失败"));
#else
    ui->textBrowser->append(tr("接收:") + pchStr);
#endif

#ifdef USE_SELF_MSGBOX
    myHelper::ShowMessageBoxError(QObject::tr("获取A级权限失败"));
#else
    QMessageBox::information(NULL, QObject::tr("提示"),
                             QObject::tr("获取A级权限失败"),
                             QMessageBox::Yes);
#endif

    ui->pushButton_45->setEnabled(true);//获取A级密钥权限
    ui->pushButton_47->setEnabled(false);//重置A级密钥
    ui->pushButton_46->setEnabled(true);//获取B级密钥权限
    ui->pushButton_48->setEnabled(false);//重置B级密钥
    ui->pushButton_49->setEnabled(false);//注销权限
    ui->pushButton_50->setEnabled(false);//制作钥匙
    ui->pushButton_55->setEnabled(false);//查看或重置钥匙编码
    ui->pushButton_56->setEnabled(false);//查看日志
    ui->pushButton_16->setEnabled(false);//恢复出厂设置

    delete[] pchStr;
    pchStr = NULL;
    delete chkSum;
    chkSum = NULL;

    return;

}

void MainWindow::on_pushButton_14_clicked()
{
#ifdef USE_SELF_MSGBOX
    int ok = myHelper::ShowMessageBoxQuesion(QObject::tr("只允许对钥匙名称和所属用户组"
                                                         "\n进行修改，确定要继续吗？"));
    if(ok != 1)
#else
    int ok = QMessageBox::warning(NULL, QObject::tr("警告"),
                                  QObject::tr("只允许对钥匙名称和所属用户组"
                                              "\n进行修改，确定要继续吗？"),
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::Yes);
    if(ok == QMessageBox::No)
#endif
    {
        return;
    }

    m_keyModel.database().transaction(); //开始事务操作
    if (m_keyModel.submitAll())
    {
        m_keyModel.database().commit(); //提交
    }
    else
    {
        m_keyModel.database().rollback(); //回滚
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("数据库错误: %1")
                                      .arg(m_keyModel.lastError().text()));
#else
        QMessageBox::warning(NULL, QObject::tr("警告"),
                             QObject::tr("数据库错误: %1")
                             .arg(m_keyModel.lastError().text()),
                             QMessageBox::Yes);
#endif
    }

    this->updateKeyTableView();
}

void MainWindow::on_pushButton_15_clicked()
{
    m_keyModel.revertAll();
}

void MainWindow::on_pushButton_59_clicked()
{
    QString passwordB = ui->lineEdit_23->text().trimmed();
    if(passwordB.isEmpty()||passwordB.isNull())
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("密钥不能为空"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("密钥不能为空"),
                                 QMessageBox::Yes);
#endif
        return;
    }

    if(passwordB.length() > 6)
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("密钥不能大于6位"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("密钥不能大于6位"),
                                 QMessageBox::Yes);
#endif
        return;
    }

    QString KEY_ADDRESS;
    if(!this->getKeyAddress(KEY_ADDRESS))
    {
        return;
    }

    char *pchStr = new char[128];
    char *chkSum = new char;
    sprintf_s(pchStr, 128, "%sG%s,", KEY_ADDRESS.toLocal8Bit().data(), passwordB.toLocal8Bit().data());
    SPComm::getChksum(pchStr, chkSum);
    sprintf_s(pchStr, 128, "#%sG%s,%02X\r\n", KEY_ADDRESS.toLocal8Bit().data(),
              passwordB.toLocal8Bit().data(), *chkSum & 0xff);
    //写串口
    qDebug() << "串口发送:" <<pchStr;

#ifndef SHOW_COM_DATA
    ui->textBrowser->append(tr("发送:获取B级密钥指令"));
#else
    ui->textBrowser->append(tr("发送:") + pchStr);
#endif

    int retWrite = SPComm::writeData(pchStr, strlen(pchStr));
    if(retWrite < 0)
    {
        delete[] pchStr;
        pchStr = NULL;
        delete chkSum;
        chkSum = NULL;

        return;
    }

    //读串口
    bool retRead = SPComm::readAllData(pchStr, 1500);
    if(retRead)
    {
        qDebug() << "串口接收:" <<pchStr;
        QString keyRet = pchStr;

        if(keyRet.contains("OK"))
        {

#ifndef SHOW_COM_DATA
            ui->textBrowser->append(tr("接收:获取B级权限成功"));
#else
            ui->textBrowser->append(tr("接收:") + pchStr);
#endif

#ifdef USE_SELF_MSGBOX
            myHelper::ShowMessageBoxInfo(QObject::tr("获取B级权限成功"));
#else
            QMessageBox::information(NULL, QObject::tr("提示"),
                                     QObject::tr("获取B级权限成功"),
                                     QMessageBox::Yes);
#endif

            ui->pushButton_47->setEnabled(true);//重置A级密钥
            ui->pushButton_48->setEnabled(true);//重置B级密钥
            ui->pushButton_50->setEnabled(true);//制作钥匙
            ui->pushButton_55->setEnabled(true);//查看或重置钥匙编码
            ui->pushButton_56->setEnabled(true);//查看日志
            ui->pushButton_16->setEnabled(true);//恢复出厂设置

            //数据库操作
            MysqlOperation::updateSKB(this->hex82IP(KEY_ADDRESS),passwordB);
            this->updateKeyTableView();

            //记录日志
            if(ui->comboBox->currentText() == tr("管理员"))
            {
                MysqlOperation::insertRecord(ui->lineEdit->text(),
                                             MysqlOperation::getkeyCodeBykeyIP(hex82IP(KEY_ADDRESS)),
                                             "NNN3NN6N",tr("获取B级密钥权限成功"));
            }
            else
            {
                MysqlOperation::insertRecord(ui->lineEdit->text(),
                                             MysqlOperation::getkeyCodeBykeyIP(hex82IP(KEY_ADDRESS)),
                                             "NNN3NNN7",tr("获取B级密钥权限成功"));
            }


            delete[] pchStr;
            pchStr = NULL;
            delete chkSum;
            chkSum = NULL;

            return;
        }
    }

#ifndef SHOW_COM_DATA
    ui->textBrowser->append(tr("接收:获取B级权限失败"));
#else
    ui->textBrowser->append(tr("接收:") + pchStr);
#endif

#ifdef USE_SELF_MSGBOX
    myHelper::ShowMessageBoxError(QObject::tr("获取B级权限失败"));
#else
    QMessageBox::information(NULL, QObject::tr("提示"),
                             QObject::tr("获取B级权限失败"),
                             QMessageBox::Yes);
#endif

    ui->pushButton_45->setEnabled(true);//获取A级密钥权限
    ui->pushButton_47->setEnabled(false);//重置A级密钥
    ui->pushButton_46->setEnabled(true);//获取B级密钥权限
    ui->pushButton_48->setEnabled(false);//重置B级密钥
    ui->pushButton_49->setEnabled(false);//注销权限
    ui->pushButton_50->setEnabled(false);//制作钥匙
    ui->pushButton_55->setEnabled(false);//查看或重置钥匙编码
    ui->pushButton_56->setEnabled(false);//查看日志
    ui->pushButton_16->setEnabled(false);//恢复出厂设置

    delete[] pchStr;
    pchStr = NULL;
    delete chkSum;
    chkSum = NULL;

    return;
}

void MainWindow::on_pushButton_58_clicked()
{
    QString oldPwd = ui->lineEdit_13->text();
    QString newPwd = ui->lineEdit_14->text();
    QString newPwd1 = ui->lineEdit_15->text();

    //检测是否为空
    if(oldPwd.isEmpty()||oldPwd.isNull())
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("原密钥不允许为空"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("原密钥不允许为空"),
                                 QMessageBox::Yes);
#endif
        return;
    }
    if(newPwd.isEmpty()||newPwd.isNull())
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("新密钥不允许为空"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("新密钥不允许为空"),
                                 QMessageBox::Yes);
#endif
        return;
    }
    if(newPwd1.isEmpty()||newPwd1.isNull())
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("再次输入新密钥不允许为空"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("再次输入新密钥不允许为空"),
                                 QMessageBox::Yes);
#endif
        return;
    }

    //检测位数是否正确
    if(oldPwd.length() > 6)
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("原密钥不允许超过6位"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("原密钥不允许超过6位"),
                                 QMessageBox::Yes);
#endif
        return;
    }
    if(newPwd.length() > 6)
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("新密钥不允许超过6位"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("新密钥不允许超过6位"),
                                 QMessageBox::Yes);
#endif
        return;
    }
    if(newPwd1.length() > 6)
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("再次输入新密钥不允许超过6位"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("再次输入新密钥不允许超过6位"),
                                 QMessageBox::Yes);
#endif
        return;
    }

    QString KEY_ADDRESS;
    if(!this->getKeyAddress(KEY_ADDRESS))
    {
        return;
    }

    char *pchStr = new char[128];
    char *chkSum = new char;
    sprintf_s(pchStr, 128, "%sWKA%s,", KEY_ADDRESS.toLocal8Bit().data(), newPwd1.toLocal8Bit().data());
    SPComm::getChksum(pchStr, chkSum);
    sprintf_s(pchStr, 128, "#%sWKA%s,%02X\r\n", KEY_ADDRESS.toLocal8Bit().data(),
              newPwd1.toLocal8Bit().data(), *chkSum & 0xff);
    //写串口
    qDebug() << "串口发送:" <<pchStr;

#ifndef SHOW_COM_DATA
    ui->textBrowser->append(tr("发送:重置A级密钥指令"));
#else
    ui->textBrowser->append(tr("发送:") + pchStr);
#endif

    int retWrite = SPComm::writeData(pchStr, strlen(pchStr));
    if(retWrite < 0)
    {
        delete[] pchStr;
        pchStr = NULL;
        delete chkSum;
        chkSum = NULL;

        return;
    }

    //读串口
    bool retRead = SPComm::readAllData(pchStr, 1500);
    if(retRead)
    {
        qDebug() << "串口接收:" <<pchStr;
        QString keyRet = pchStr;

        if(keyRet.contains("OK"))
        {
#ifndef SHOW_COM_DATA
            ui->textBrowser->append(tr("接收:重置A级权限密钥成功"));
#else
            ui->textBrowser->append(tr("接收:") + pchStr);
#endif

#ifdef USE_SELF_MSGBOX
            myHelper::ShowMessageBoxInfo(QObject::tr("重置A级权限密钥成功"));
#else
            QMessageBox::information(NULL, QObject::tr("提示"),
                                     QObject::tr("重置A级权限密钥成功"),
                                     QMessageBox::Yes);
#endif


            //数据库操作
            MysqlOperation::updateSKA(this->hex82IP(KEY_ADDRESS),newPwd);
            this->updateKeyTableView();

            //记录日志
            if(ui->comboBox->currentText() == tr("管理员"))
            {
                MysqlOperation::insertRecord(ui->lineEdit->text(),
                                             MysqlOperation::getkeyCodeBykeyIP(hex82IP(KEY_ADDRESS)),
                                             "NNN3NN6N","重置A级密钥成功");
            }
            else
            {
                MysqlOperation::insertRecord(ui->lineEdit->text(),
                                             MysqlOperation::getkeyCodeBykeyIP(hex82IP(KEY_ADDRESS)),
                                             "NNN3NNN7","重置A级密钥成功");
            }

            delete[] pchStr;
            pchStr = NULL;
            delete chkSum;
            chkSum = NULL;

            return;
        }
    }

#ifndef SHOW_COM_DATA
    ui->textBrowser->append(tr("接收:重置A级权限密钥失败"));
#else
    ui->textBrowser->append(tr("接收:") + pchStr);
#endif

#ifdef USE_SELF_MSGBOX
    myHelper::ShowMessageBoxError(QObject::tr("重置A级权限密钥失败"));
#else
    QMessageBox::information(NULL, QObject::tr("提示"),
                             QObject::tr("重置A级权限密钥失败"),
                             QMessageBox::Yes);
#endif

    delete[] pchStr;
    pchStr = NULL;
    delete chkSum;
    chkSum = NULL;

    return;
}

void MainWindow::on_pushButton_60_clicked()
{
    QString oldPwd = ui->lineEdit_25->text();
    QString newPwd = ui->lineEdit_24->text();
    QString newPwd1 = ui->lineEdit_26->text();

    //检测是否为空
    if(oldPwd.isEmpty()||oldPwd.isNull())
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("原密钥不允许为空"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("原密钥不允许为空"),
                                 QMessageBox::Yes);
#endif
        return;
    }
    if(newPwd.isEmpty()||newPwd.isNull())
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("新密钥不允许为空"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("新密钥不允许为空"),
                                 QMessageBox::Yes);
#endif
        return;
    }
    if(newPwd1.isEmpty()||newPwd1.isNull())
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("再次输入新密钥不允许为空"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("再次输入新密钥不允许为空"),
                                 QMessageBox::Yes);
#endif
        return;
    }

    //检测位数是否正确
    if(oldPwd.length() > 6)
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("原密钥不允许超过6位"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("原密钥不允许超过6位"),
                                 QMessageBox::Yes);
#endif
        return;
    }
    if(newPwd.length() > 6)
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("新密钥不允许超过6位"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("新密钥不允许超过6位"),
                                 QMessageBox::Yes);
#endif
        return;
    }
    if(newPwd1.length() > 6)
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("再次输入新密钥不允许超过6位"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("再次输入新密钥不允许超过6位"),
                                 QMessageBox::Yes);
#endif
        return;
    }

    QString KEY_ADDRESS;
    if(!this->getKeyAddress(KEY_ADDRESS))
    {
        return;
    }

    char *pchStr = new char[128];
    char *chkSum = new char;
    sprintf_s(pchStr, 128, "%sWKB%s,", KEY_ADDRESS.toLocal8Bit().data(), newPwd1.toLocal8Bit().data());
    SPComm::getChksum(pchStr, chkSum);
    sprintf_s(pchStr, 128, "#%sWKB%s,%02X\r\n", KEY_ADDRESS.toLocal8Bit().data(),
              newPwd1.toLocal8Bit().data(), *chkSum & 0xff);
    //写串口
    qDebug() << "串口发送:" <<pchStr;

#ifndef SHOW_COM_DATA
    ui->textBrowser->append(tr("发送:重置B级密钥指令"));
#else
    ui->textBrowser->append(tr("发送:") + pchStr);
#endif

    int retWrite = SPComm::writeData(pchStr, strlen(pchStr));
    if(retWrite < 0)
    {
        delete[] pchStr;
        pchStr = NULL;
        delete chkSum;
        chkSum = NULL;

        return;
    }

    //读串口
    bool retRead = SPComm::readAllData(pchStr, 1500);
    if(retRead)
    {
        qDebug() << "串口接收:" <<pchStr;
        QString keyRet = pchStr;

        if(keyRet.contains("OK"))
        {

#ifndef SHOW_COM_DATA
            ui->textBrowser->append(tr("接收:重置B级密钥成功"));
#else
            ui->textBrowser->append(tr("接收:") + pchStr);
#endif

#ifdef USE_SELF_MSGBOX
            myHelper::ShowMessageBoxInfo(QObject::tr("重置B级权限密钥成功"));
#else
            QMessageBox::information(NULL, QObject::tr("提示"),
                                     QObject::tr("重置B级权限密钥成功"),
                                     QMessageBox::Yes);
#endif

            //数据库操作
            MysqlOperation::updateSKB(this->hex82IP(KEY_ADDRESS),newPwd);
            this->updateKeyTableView();

            //记录日志
            if(ui->comboBox->currentText() == tr("管理员"))
            {
                MysqlOperation::insertRecord(ui->lineEdit->text(),
                                             MysqlOperation::getkeyCodeBykeyIP(hex82IP(KEY_ADDRESS)),
                                             "NNN3NN6N","重置B级密钥成功");
            }
            else
            {
                MysqlOperation::insertRecord(ui->lineEdit->text(),
                                             MysqlOperation::getkeyCodeBykeyIP(hex82IP(KEY_ADDRESS)),
                                             "NNN3NNN7","重置B级密钥成功");
            }

            delete[] pchStr;
            pchStr = NULL;
            delete chkSum;
            chkSum = NULL;

            return;
        }
    }

#ifndef SHOW_COM_DATA
    ui->textBrowser->append(tr("接收:重置B级密钥失败"));
#else
    ui->textBrowser->append(tr("接收:") + pchStr);
#endif

#ifdef USE_SELF_MSGBOX
    myHelper::ShowMessageBoxError(QObject::tr("重置B级权限密钥失败"));
#else
    QMessageBox::information(NULL, QObject::tr("提示"),
                             QObject::tr("重置B级权限密钥失败"),
                             QMessageBox::Yes);
#endif

    delete[] pchStr;
    pchStr = NULL;
    delete chkSum;
    chkSum = NULL;

    return;
}

void MainWindow::on_pushButton_61_clicked()
{
    QString sTime = ui->dateEdit->date().toString("yyyyMMdd");
    QString eTime = ui->dateEdit_2->date().toString("yyyyMMdd");
    qDebug() << "权限开始日期:" << sTime;
    qDebug() << "权限截止日期:" << eTime;

    QString keyMCIP = ui->lineEdit_27->text();
    qDebug() << "钥匙权限掩码:" << keyMCIP;
    if(this->getKeyEmpowerStatus(keyMCIP) == -1)
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("权限掩码不符合规范"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("权限掩码不符合规范"),
                                 QMessageBox::Yes);
#endif
        return;
    }

    QString keyMCAddress = this->IP28hex(keyMCIP);
    qDebug() << "钥匙权限掩码转8Hex:" << keyMCAddress;

    QString addrSTET = keyMCAddress + "," + sTime + "," + eTime;

    QString KEY_ADDRESS;
    if(!this->getKeyAddress(KEY_ADDRESS))
    {
        return;
    }

    char *pchStr = new char[128];
    char *chkSum = new char;
    sprintf_s(pchStr, 128, "%sS%s,", KEY_ADDRESS.toLocal8Bit().data(), addrSTET.toLocal8Bit().data());
    SPComm::getChksum(pchStr, chkSum);
    sprintf_s(pchStr, 128, "#%sS%s,%02X\r\n", KEY_ADDRESS.toLocal8Bit().data(),
              addrSTET.toLocal8Bit().data(), *chkSum & 0xff);
    //写串口
    qDebug() << "串口发送:" <<pchStr;

#ifndef SHOW_COM_DATA
    ui->textBrowser->append(tr("发送:修改钥匙权限掩码指令"));
#else
    ui->textBrowser->append(tr("发送:") + pchStr);
#endif

    int retWrite = SPComm::writeData(pchStr, strlen(pchStr));
    if(retWrite < 0)
    {
        delete[] pchStr;
        pchStr = NULL;
        delete chkSum;
        chkSum = NULL;

        return;
    }

    //读串口
    bool retRead = SPComm::readAllData(pchStr, 1500);
    if(retRead)
    {
        qDebug() << "串口接收:" <<pchStr;
        QString keyRet = pchStr;
        if(keyRet.contains("OK"))
        {
#ifndef SHOW_COM_DATA
            ui->textBrowser->append(tr("接收:修改钥匙权限掩码成功"));
#else
            ui->textBrowser->append(tr("接收:") + pchStr);
#endif

#ifdef USE_SELF_MSGBOX
            myHelper::ShowMessageBoxInfo(QObject::tr("制作钥匙成功"));
#else
            QMessageBox::information(NULL, QObject::tr("提示"),
                                     QObject::tr("制作钥匙成功"),
                                     QMessageBox::Yes);
#endif

            //数据库操作
            QString empower;
            int ret = this->getKeyEmpowerStatus(keyMCIP);
            switch(ret)
            {
            case 0:
                empower = tr("1开1");
                break;
            case 1:
                empower = tr("1开254");
                break;
            case 2:
                empower = tr("1开254^2");
                break;
            case 3:
                empower = tr("1开254^3");
                break;
            case 4:
                empower = tr("全开");
                break;
            default:
                break;
            }

            //数据库操作
            MysqlOperation::updateEmpower(this->hex82IP(KEY_ADDRESS),empower,
                                          keyMCIP,ui->dateEdit->date().toString("yyyy/MM/dd"),
                                          ui->dateEdit_2->date().toString("yyyy/MM/dd"));
            this->updateKeyTableView();

            //记录日志
            if(ui->comboBox->currentText() == tr("管理员"))
            {
                MysqlOperation::insertRecord(ui->lineEdit->text(),
                                             MysqlOperation::getkeyCodeBykeyIP(hex82IP(KEY_ADDRESS)),
                                             "NNN3NN6N",tr("修改钥匙权限成功,权限修改为:%1").arg(empower));
            }
            else
            {
                MysqlOperation::insertRecord(ui->lineEdit->text(),
                                             MysqlOperation::getkeyCodeBykeyIP(hex82IP(KEY_ADDRESS)),
                                             "NNN3NNN7",tr("修改钥匙权限成功,权限修改为:%1").arg(empower));
            }

            delete[] pchStr;
            pchStr = NULL;
            delete chkSum;
            chkSum = NULL;

            return;
        }
    }

#ifndef SHOW_COM_DATA
    ui->textBrowser->append(tr("接收:修改钥匙权限掩码失败"));
#else
    ui->textBrowser->append(tr("接收:") + pchStr);
#endif

#ifdef USE_SELF_MSGBOX
    myHelper::ShowMessageBoxError(QObject::tr("制作钥匙失败"));
#else
    QMessageBox::information(NULL, QObject::tr("提示"),
                             QObject::tr("制作钥匙失败"),
                             QMessageBox::Yes);
#endif

    delete[] pchStr;
    pchStr = NULL;
    delete chkSum;
    chkSum = NULL;

    return;
}

void MainWindow::on_pushButton_66_clicked()
{
    qDebug() << "新钥匙编码IP形式:" << ui->lineEdit_33->text();
    if(ui->lineEdit_33->text() == ui->lineEdit_32->text())
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("新钥匙编码不能和原锁编码相同"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("新钥匙编码不能和原锁编码相同"),
                                 QMessageBox::Yes);
#endif
        return;
    }
    QString newKeyAddress = this->IP28hex(ui->lineEdit_33->text());
    qDebug() << "新钥匙编码:" << newKeyAddress;

    if(newKeyAddress.isEmpty()||newKeyAddress.isNull())
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("新钥匙编码不允许为空"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("新钥匙编码不允许为空"),
                                 QMessageBox::Yes);
#endif
        return;
    }
#ifdef USE_SELF_MSGBOX
    int ok = myHelper::ShowMessageBoxQuesion(QObject::tr("是否将钥匙编码:%1\n改成%2？")
                                    .arg(ui->lineEdit_32->text(), ui->lineEdit_33->text()));
    if(ok != 1)
#else
    int ok = QMessageBox::warning(NULL, QObject::tr("警告"),
                                  QObject::tr("是否将钥匙编码:%1\n改成%2？")
                                  .arg(ui->lineEdit_32->text(), ui->lineEdit_33->text()),
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::Yes);
    if(ok == QMessageBox::No)
#endif
    {
        return;
    }

    QString KEY_ADDRESS = this->IP28hex(ui->lineEdit_32->text());

    char *pchStr = new char[128];
    char *chkSum = new char;
    sprintf_s(pchStr, 128, "%sWKC%s,", KEY_ADDRESS.toLocal8Bit().data(),
              newKeyAddress.toLocal8Bit().data());
    SPComm::getChksum(pchStr, chkSum);
    sprintf_s(pchStr, 128, "#%sWKC%s,%02X\r", KEY_ADDRESS.toLocal8Bit().data(),
              newKeyAddress.toLocal8Bit().data(), *chkSum & 0xff);
    //写串口
    qDebug() << "串口发送:" <<pchStr;

#ifndef SHOW_COM_DATA
    ui->textBrowser->append(tr("发送:重置钥匙编码指令"));
#else
    ui->textBrowser->append(tr("发送:") + pchStr);
#endif

    int retWrite = SPComm::writeData(pchStr, strlen(pchStr));
    if(retWrite < 0)
    {
        delete[] pchStr;
        pchStr = NULL;
        delete chkSum;
        chkSum = NULL;

        return;
    }

    //读串口
    bool retRead = SPComm::readAllData(pchStr, 1500);
    if(retRead)
    {
        qDebug() << "串口接收:" <<pchStr;
        QString retKey = pchStr;

        if(retKey.contains("OK"))
        {

#ifndef SHOW_COM_DATA
            ui->textBrowser->append(tr("接收:重置钥匙编码成功"));
#else
            ui->textBrowser->append(tr("接收:") + pchStr);
#endif

#ifdef USE_SELF_MSGBOX
            myHelper::ShowMessageBoxInfo(QObject::tr("修改钥匙编码成功"));
#else
            QMessageBox::information(NULL, QObject::tr("提示"),
                                     QObject::tr("修改钥匙编码成功"),
                                     QMessageBox::Yes);
#endif

            //数据库操作
            MysqlOperation::updateKeyIP(ui->lineEdit_32->text().trimmed(),
                                          ui->lineEdit_33->text().trimmed());
            this->updateKeyTableView();

            if(ui->comboBox->currentText() == tr("管理员"))
            {
                MysqlOperation::insertRecord(ui->lineEdit->text(),
                                             MysqlOperation::getkeyCodeBykeyIP(hex82IP(KEY_ADDRESS)),
                                             "NNN3NN6N",tr("修改钥匙编码成功"));
            }
            else
            {
                MysqlOperation::insertRecord(ui->lineEdit->text(),
                                             MysqlOperation::getkeyCodeBykeyIP(hex82IP(KEY_ADDRESS)),
                                             "NNN3NNN7",tr("修改钥匙编码成功"));
            }

            ui->lineEdit_32->setText(ui->lineEdit_33->text());

            delete[] pchStr;
            pchStr = NULL;
            delete chkSum;
            chkSum = NULL;

            return;
        }
    }

#ifndef SHOW_COM_DATA
    ui->textBrowser->append(tr("接收:重置钥匙编码失败"));
#else
    ui->textBrowser->append(tr("接收:") + pchStr);
#endif

#ifdef USE_SELF_MSGBOX
    myHelper::ShowMessageBoxError(QObject::tr("修改钥匙编码失败"));
#else
    QMessageBox::information(NULL, QObject::tr("提示"),
                             QObject::tr("修改钥匙编码失败"),
                             QMessageBox::Yes);
#endif

    delete[] pchStr;
    pchStr = NULL;
    delete chkSum;
    chkSum = NULL;

    return;
}

void MainWindow::on_pushButton_21_clicked()
{
    QFile file("ServerConfig.ini");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&file);
        out << ui->spinBox->text().trimmed() << "\n"; //连接数
        out << ui->spinBox_2->text().trimmed() << "\n"; //端口号
        if(ui->checkBox->isChecked())
        {
            out << "1" << "\n"; //服务器自动启动
        }
        else
        {
            out << "0" << "\n"; //服务器手动启动
        }

        file.close();
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxInfo(QObject::tr("保存设置成功"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
        QObject::tr("保存设置成功"), QMessageBox::Yes);
#endif
    }
}

void MainWindow::on_pushButton_72_clicked()
{
    if(m_startServer)
    {
        disconnect(this->m_listenSocket,SIGNAL(newConnection()),
                   this,SLOT(myslot_processConnection()));
        m_mapNetIsKey.clear();
        m_mapDevCodeIsKey.clear();
        m_listenSocket->close();

        delete m_listenSocket;
        m_listenSocket = NULL;

        if(ui->comboBox->currentText() == tr("操作员"))
        {
             MysqlOperation::insertRecord(ui->lineEdit->text().trimmed(),"","NNNNN56N",QObject::tr("关闭服务器成功"));
        }
        else
        {
            MysqlOperation::insertRecord(ui->lineEdit->text().trimmed(),"","NNNNN5N7",QObject::tr("关闭服务器成功"));
        }

        ui->pushButton_72->setText(QObject::tr("启动服务器"));
        ui->pushButton_72->setIcon(QIcon(":/image/ball_off.ico"));
        m_currentConetCount = 0;
        this->currentNumLabel->setText(QObject::tr("当前连接数:<font color='red'>%1</font>")
                                       .arg(QString::number(m_currentConetCount)));
        serStatusLabel->setText(QObject::tr("服务器状态:<font color='red'>%1</font>").arg(QString("已关闭")));
        m_startServer = false;        
    }
    else
    {
        if(!this->startServer())
        {
#ifdef USE_SELF_MSGBOX
            myHelper::ShowMessageBoxError(QObject::tr("启动服务器失败"));
#else
            QMessageBox::warning(NULL, QObject::tr("警告"),
            QObject::tr("启动服务器失败"), QMessageBox::Yes);
#endif
        }
    }
}

void MainWindow::on_pushButton_75_clicked()
{
    this->updateLockTableWidget();
}


void MainWindow::on_pushButton_2_clicked()
{
    this->updateAdminInfo();
}

void MainWindow::on_pushButton_clicked()
{
    QVector<QString> addminInfoVector;
    addminInfoVector.append(ui->lineEdit->text());//姓名
    addminInfoVector.append(ui->lineEdit_2->text());//登录名
    addminInfoVector.append(ui->lineEdit_3->text());//密码
    addminInfoVector.append(ui->comboBox->currentText());//权限组
    addminInfoVector.append(ui->comboBox_13->currentText());//用户组
    addminInfoVector.append(ui->lineEdit_5->text());//联系方式
    addminInfoVector.append(ui->comboBox_2->currentText());//权限状态
    addminInfoVector.append(ui->dateTimeEdit->dateTime().toString("yyyy/MM/dd hh:mm"));//权限起始时间
    addminInfoVector.append(ui->dateTimeEdit_2->dateTime().toString("yyyy/MM/dd hh:mm"));//权限截止时间

    if(MysqlOperation::updateAdminInfoByLogName(m_logName,addminInfoVector))
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxInfo(QObject::tr("保存成功"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
        QObject::tr("保存成功"), QMessageBox::Yes);
#endif
        this->updateAdminTableView();
    }
    else
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("保存失败"));
#else
        QMessageBox::warning(NULL, QObject::tr("警告"),
        QObject::tr("保存失败"), QMessageBox::Yes);
#endif
    }
}

bool g_flagAllSelected = false;
void MainWindow::on_pushButton_37_clicked()
{
    if(!g_flagAllSelected)
    {
        int rowCount = ui->tableWidget_2->rowCount();
        for(int row=0; row < rowCount; row++)
        {
            ((QCheckBox*)(ui->tableWidget_2->cellWidget(row,0)))->setChecked(true);
        }

        ui->pushButton_37->setText(tr("全不选"));
        ui->pushButton_37->setIcon(QIcon(":/image/checkbox_checked.png"));
        g_flagAllSelected = true;
    }
    else
    {
        int rowCount = ui->tableWidget_2->rowCount();
        for(int row=0; row < rowCount; row++)
        {
            ((QCheckBox*)(ui->tableWidget_2->cellWidget(row,0)))->setChecked(false);
        }

        ui->pushButton_37->setText(tr("全选"));
         ui->pushButton_37->setIcon(QIcon(":/image/checkbox_unchecked.png"));
        g_flagAllSelected = false;
    }
}

void MainWindow::on_pushButton_36_clicked()
{
    QString lockUser = ui->lineEdit_16->text().trimmed();
    QString password = ui->lineEdit_17->text().trimmed();
    QString password2 = ui->lineEdit_18->text().trimmed();

    if(lockUser.isEmpty() || lockUser.isNull())
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("用户名不能为空"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("用户名不能为空"),
                                 QMessageBox::Yes);
#endif
        return;
    }
    if(lockUser.trimmed().length() > 6)
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("用户名不能超过6位"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("用户名不能超过6位"),
                                 QMessageBox::Yes);
#endif
        return;
    }
    if(password.isEmpty() || password.isNull())
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("密码不能为空"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("密码不能为空"),
                                 QMessageBox::Yes);
#endif
        return;
    }
    if(password.length() > 6)
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("密码不能超过6位"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("密码不能超过6位"),
                                 QMessageBox::Yes);
#endif
        return;
    }
    if(password2.isEmpty() || password2.isNull())
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("确认密码不能为空"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("确认密码不能为空"),
                                 QMessageBox::Yes);
#endif
        return;
    }
    if(password2.length() > 6)
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("确认密码不能超过6位"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("确认密码不能超过6位"),
                                 QMessageBox::Yes);
#endif
        return;
    }

    if(password != password2)
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("密码不一致"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("密码不一致"),
                                 QMessageBox::Yes);
#endif
        return;
    }

    if(!m_startServer)
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxInfo(QObject::tr("请先启动服务器"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("请先启动服务器"),
                                 QMessageBox::Yes);
#endif
        return;
    }

    ui->pushButton_36->setEnabled(false);

    bool atLeastOneSelFlag = false;
    char *pchStr = new char[128];
    char *chkSum = new char;
    bool isAddOk = false;
    for(int row=0; row < ui->tableWidget_2->rowCount(); row++)
    {
        if(((QCheckBox*)(ui->tableWidget_2->cellWidget(row, 0)))->isChecked())
        {
            atLeastOneSelFlag = true;
            //发送
            QString lockCode = ui->tableWidget_2->item(row, 1)->text();
            if(!(m_mapDevCodeIsKey.contains(lockCode)))
            {
#ifdef USE_SELF_MSGBOX
                myHelper::ShowMessageBoxError(QObject::tr("编号为%1的锁具未连接到服务器!")
                                              .arg(lockCode));
#else
                QMessageBox::warning(NULL, QObject::tr("警告"),
                                     QObject::tr("编号为%1的锁具未连接到服务器!")
                                     .arg(lockCode),
                                     QMessageBox::Yes);
#endif
                continue;
            }

            QTcpSocket *lockSocket = m_mapDevCodeIsKey.value(lockCode);
            QString lockHexAddr;
            if(this->getLockAddress(lockSocket,lockHexAddr))
            {
#ifdef USE_SELF_MSGBOX
                myHelper::ShowMessageBoxError(QObject::tr("获取编号为%1的锁具ID失败!")
                                              .arg(lockCode));
#else
                QMessageBox::warning(NULL, QObject::tr("警告"),
                                     QObject::tr("获取编号为%1的锁具ID失败!")
                                     .arg(lockCode),
                                     QMessageBox::Yes);
#endif
                continue;
            }

            if(!MysqlOperation::isLockIPTheSame(lockCode,this->hex82IP(lockHexAddr)))
            {
#ifdef USE_SELF_MSGBOX
               int ok = myHelper::ShowMessageBoxQuesion(QObject::tr("编号为%1的锁具ID与数据库中的不一致，\n是否更新？")
                                                        .arg(lockCode));
               if(ok == 1)
#else
               int ok = QMessageBox::information(NULL, QObject::tr("提示"),
                                         QObject::tr("编号为%1的锁具ID与数据库中的不一致，\n是否更新？")
                                         .arg(lockCode),
                                         QMessageBox::Yes | QMessageBox::No,
                                         QMessageBox::Yes);

               if(ok == QMessageBox::Yes)
#endif
               {
                   MysqlOperation::updateLockIP(lockCode,this->hex82IP(lockHexAddr));
               }
            }

            sprintf_s(pchStr, 128, "%sWU%s,%s,", lockHexAddr.toLocal8Bit().data(),
                      lockUser.toLocal8Bit().data(), password.toLocal8Bit().data());
            SPComm::getChksum(pchStr, chkSum);
            sprintf_s(pchStr, 128, "#%sWU%s,%s,%02X\r", lockHexAddr.toLocal8Bit().data(),
                      lockUser.toLocal8Bit().data(), password.toLocal8Bit().data(), *chkSum & 0xff);

            QByteArray readBuf;
            if(this->socketWriteAndRead(lockSocket,pchStr,readBuf))
            {
                if(!(readBuf.contains("OK")))
                {
                    isAddOk = false;
#ifdef USE_SELF_MSGBOX
                    myHelper::ShowMessageBoxError(QObject::tr("编号为%1的锁具，\n添加锁用户失败")
                                                  .arg(lockCode));
#else
                    QMessageBox::warning(NULL, QObject::tr("警告"),
                                         QObject::tr("编号为%1的锁具，\n添加锁用户失败")
                                         .arg(lockCode),
                                         QMessageBox::Yes);
#endif
                }
                else
                {
                    isAddOk = true;
                }
            }

        }//end if
    }//end for

    if(!atLeastOneSelFlag)
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxInfo(QObject::tr("请选择要添加锁用户的锁具"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("请选择要添加锁用户的锁具"),
                                 QMessageBox::Yes);
#endif
    }
    else
    {
        if(isAddOk)
        {
            //记录日志
            if(ui->comboBox->currentText() == tr("管理员"))
            {
                MysqlOperation::insertRecord(ui->lineEdit->text(),
                                             "",
                                             "NNNN4N6N",tr("增加锁用户成功"));
            }
            else
            {
                MysqlOperation::insertRecord(ui->lineEdit->text(),
                                             "",
                                             "NNNN4NN7",tr("增加锁用户成功"));
            }
        }
    }

    delete[] pchStr;
    pchStr = NULL;
    delete chkSum;
    chkSum = NULL;

    ui->pushButton_36->setEnabled(true);
}

void MainWindow::on_pushButton_16_clicked()
{
    QString KEY_ADDRESS;
    if(!this->getKeyAddress(KEY_ADDRESS))
    {
        return;
    }

    char *pchStr = new char[128];
    char *chkSum = new char;
    sprintf_s(pchStr, 128, "%sRS,", KEY_ADDRESS.toLocal8Bit().data());
    SPComm::getChksum(pchStr, chkSum);
    sprintf_s(pchStr, 128, "#%sRS,%02X\r\n", KEY_ADDRESS.toLocal8Bit().data(),*chkSum & 0xff);
    //写串口
    qDebug() << "串口发送:" << pchStr;

#ifndef SHOW_COM_DATA
    ui->textBrowser->append(tr("发送:恢复出厂设置指令"));
#else
    ui->textBrowser->append(tr("发送:") + pchStr);
#endif

    int retWrite = SPComm::writeData(pchStr, strlen(pchStr));
    if(retWrite < 0)
    {
        delete[] pchStr;
        pchStr = NULL;
        delete chkSum;
        chkSum = NULL;

        return;
    }

    //读串口
    bool retRead = SPComm::readAllData(pchStr, 1500);
    if(retRead)
    {
        qDebug() << "串口接收:" <<pchStr;
        ui->textBrowser->append(tr("接收:") + pchStr);
        QString keyRet = pchStr;

        if(keyRet.contains("OK"))
        {
#ifndef SHOW_COM_DATA
            ui->textBrowser->append(tr("发送:恢复出厂设置成功"));
#else
            ui->textBrowser->append(tr("发送:") + pchStr);
#endif

#ifdef USE_SELF_MSGBOX
            myHelper::ShowMessageBoxInfo(QObject::tr("恢复出厂设置成功"));
#else
            QMessageBox::information(NULL, QObject::tr("提示"),
                                     QObject::tr("恢复出厂设置成功"),
                                     QMessageBox::Yes);
#endif

            //数据库操作
            MysqlOperation::updateSKA(this->hex82IP(KEY_ADDRESS),"555888");
            MysqlOperation::updateSKB(this->hex82IP(KEY_ADDRESS),"ETON58");
            MysqlOperation::updateKeyIP(this->hex82IP(KEY_ADDRESS),"192.128.64.1");
            this->updateKeyTableView();

            //记录日志
            if(ui->comboBox->currentText() == tr("管理员"))
            {
                MysqlOperation::insertRecord(ui->lineEdit->text(),
                                             MysqlOperation::getkeyCodeBykeyIP(hex82IP(KEY_ADDRESS)),
                                             "NNN3NN6N",tr("恢复出厂设置成功"));
            }
            else
            {
                MysqlOperation::insertRecord(ui->lineEdit->text(),
                                             MysqlOperation::getkeyCodeBykeyIP(hex82IP(KEY_ADDRESS)),
                                             "NNN3NNN7",tr("恢复出厂设置成功"));
            }

            delete[] pchStr;
            pchStr = NULL;
            delete chkSum;
            chkSum = NULL;

            return;
        }
    }

#ifndef SHOW_COM_DATA
    ui->textBrowser->append(tr("发送:恢复出厂设置失败"));
#else
    ui->textBrowser->append(tr("发送:") + pchStr);
#endif

#ifdef USE_SELF_MSGBOX
    myHelper::ShowMessageBoxError(QObject::tr("恢复出厂设置失败"));
#else
    QMessageBox::information(NULL, QObject::tr("提示"),
                             QObject::tr("恢复出厂设置失败"),
                             QMessageBox::Yes);
#endif

    delete[] pchStr;
    pchStr = NULL;
    delete chkSum;
    chkSum = NULL;

    return;
}

int g_currentRow = 0;
bool g_atLeastOneSelFlag = false;
void MainWindow::on_pushButton_81_clicked()
{
    while( g_currentRow < ui->tableWidget_2->rowCount() )
    {
        if(((QCheckBox*)(ui->tableWidget_2->cellWidget(g_currentRow, 0)))->isChecked())
        {
            g_atLeastOneSelFlag = true;
            ui->tableWidget_2->selectRow(g_currentRow);

            ui->lineEdit_19->setText(ui->tableWidget_2->item(g_currentRow, 7)->text().trimmed());

            g_currentRow++;

            if(g_currentRow == ui->tableWidget_2->rowCount())
            {
                g_currentRow = 0;
            }

            break;
        }

         g_currentRow++;

         if(g_currentRow == ui->tableWidget_2->rowCount())
         {
             if(!g_atLeastOneSelFlag)
             {
#ifdef USE_SELF_MSGBOX
                 myHelper::ShowMessageBoxInfo(QObject::tr("请选择要重置的锁具"));
#else
                 QMessageBox::information(NULL, QObject::tr("提示"),
                                          QObject::tr("请选择要重置的锁具"),
                                          QMessageBox::Yes);
#endif
             }

             g_currentRow = 0;
             g_atLeastOneSelFlag = false;
             break;
         }
    }
}

void MainWindow::on_pushButton_86_clicked()
{
    QString newLC = ui->lineEdit_20->text().trimmed();

    qDebug() << tr("newLC is : ") << newLC;

    if(newLC == "...")
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("新锁ID不能为空"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("新锁ID不能为空"),
                                 QMessageBox::Yes);
#endif
        return;
    }

    QString newLcHex = this->IP28hex(newLC);

    if(!m_startServer)
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxInfo(QObject::tr("请先启动服务器"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("请先启动服务器"),
                                 QMessageBox::Yes);
#endif
        return;
    }

    //发送
    QString lockCode = ui->tableWidget_2->item(g_currentRow - 1, 7)->text();
    if(!(m_mapDevCodeIsKey.contains(lockCode)))
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("编号为%1的锁具未连接到服务器!")
                                      .arg(lockCode));
#else
        QMessageBox::warning(NULL, QObject::tr("警告"),
                             QObject::tr("编号为%1的锁具未连接到服务器!")
                             .arg(lockCode),
                             QMessageBox::Yes);
#endif
        return;
    }

    QTcpSocket *lockSocket = m_mapDevCodeIsKey.value(lockCode);
    QString lockHexAddr;
    if(this->getLockAddress(lockSocket,lockHexAddr))
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("获取编号为%1的锁具ID失败!")
                                      .arg(lockCode));
#else
        QMessageBox::warning(NULL, QObject::tr("警告"),
                             QObject::tr("获取编号为%1的锁具ID失败!")
                             .arg(lockCode),
                             QMessageBox::Yes);
#endif
        return;
    }

    if(!MysqlOperation::isLockIPTheSame(lockCode,this->hex82IP(lockHexAddr)))
    {
#ifdef USE_SELF_MSGBOX
        int ok = myHelper::ShowMessageBoxQuesion(QObject::tr("编号为%1的锁具ID与数据库中的不一致，\n是否更新？")
                                                 .arg(lockCode));
        if(ok == 1)
#else
        int ok = QMessageBox::information(NULL, QObject::tr("提示"),
                                          QObject::tr("编号为%1的锁具ID与数据库中的不一致，\n是否更新？")
                                          .arg(lockCode),
                                          QMessageBox::Yes | QMessageBox::No,
                                          QMessageBox::Yes);

        if(ok == QMessageBox::Yes)
#endif
        {
            MysqlOperation::updateLockIP(lockCode,this->hex82IP(lockHexAddr));
        }
    }

    char *pchStr = new char[128];
    char *chkSum = new char;

    sprintf_s(pchStr, 128, "FFFFFFFFWA%s,", newLcHex.toLocal8Bit().data());
    SPComm::getChksum(pchStr, chkSum);
    sprintf_s(pchStr, 128, "#FFFFFFFFWA%s,%02X\r", newLcHex.toLocal8Bit().data(), *chkSum & 0xff);

    QByteArray readBuf;
    if(this->socketWriteAndRead(lockSocket,pchStr,readBuf))
    {
        if(!(readBuf.contains("OK")))
        {
#ifdef USE_SELF_MSGBOX
            myHelper::ShowMessageBoxError(QObject::tr("编号为%1的锁具，\n重置锁ID失败")
                                          .arg(lockCode));
#else
            QMessageBox::warning(NULL, QObject::tr("警告"),
                                 QObject::tr("编号为%1的锁具，\n重置锁ID失败")
                                 .arg(lockCode),
                                 QMessageBox::Yes);
#endif
        }
        else
        {
            ui->tableWidget_2->item(g_currentRow - 1, 7)->setText(newLC);
            MysqlOperation::updateLockIP(lockCode,newLC);
            this->updateLockTableView();

            //记录日志
            if(ui->comboBox->currentText() == tr("管理员"))
            {
                MysqlOperation::insertRecord(ui->lineEdit->text(),
                                            lockCode,
                                             "NNNN4N6N",tr("重置锁ID成功"));
            }
            else
            {
                MysqlOperation::insertRecord(ui->lineEdit->text(),
                                             lockCode,
                                             "NNNN4NN7",tr("重置锁ID成功"));
            }

#ifdef USE_SELF_MSGBOX
            myHelper::ShowMessageBoxInfo(QObject::tr("编号为%1的锁具，\n重置锁ID成功")
                                         .arg(lockCode));
#else
            QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("编号为%1的锁具，\n重置锁ID成功")
                                 .arg(lockCode),
                                 QMessageBox::Yes);
#endif
        }
    }

    delete[] pchStr;
    pchStr = NULL;
    delete chkSum;
    chkSum = NULL;
}

void MainWindow::on_pushButton_34_clicked()
{
#if 0
    if(!m_startServer)
    {
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("请先启动服务器"),
                                 QMessageBox::Yes);
        return;
    }

    ui->pushButton_34->setEnabled(false);

    bool atLeastOneSelFlag = false;
    char *pchStr = new char[128];
    char *chkSum = new char;
    for(int row=0; row < ui->tableWidget_2->rowCount(); row++)
    {
        if(((QCheckBox*)(ui->tableWidget_2->cellWidget(row, 0)))->isChecked())
        {
            atLeastOneSelFlag = true;
            ui->tableWidget_2->selectRow(row);
            //发送
            QString lockCode = ui->tableWidget_2->item(row, 1)->text();
            if(!(m_mapDevCodeIsKey.contains(lockCode)))
            {
                QMessageBox::warning(NULL, QObject::tr("警告"),
                                     QObject::tr("编号为%1的锁具未连接到服务器!")
                                     .arg(lockCode),
                                     QMessageBox::Yes);
                continue;
            }

            QTcpSocket *lockSocket = m_mapDevCodeIsKey.value(lockCode);
            QString lockHexAddr;
            if(this->getLockAddress(lockSocket,lockHexAddr))
            {
                QMessageBox::warning(NULL, QObject::tr("警告"),
                                     QObject::tr("获取编号为%1的锁具ID失败!")
                                     .arg(lockCode),
                                     QMessageBox::Yes);
                continue;
            }

            if(!MysqlOperation::isLockIPTheSame(lockCode,this->hex82IP(lockHexAddr)))
            {
               int ok = QMessageBox::information(NULL, QObject::tr("提示"),
                                         QObject::tr("编号为%1的锁具ID与数据库中的不一致，\n是否更新？")
                                         .arg(lockCode),
                                         QMessageBox::Yes | QMessageBox::No,
                                         QMessageBox::Yes);

               if(ok == QMessageBox::Yes)
               {
                   MysqlOperation::updateLockIP(lockCode,this->hex82IP(lockHexAddr));
               }
            }

            QDateTime dateTime = QDateTime::currentDateTime();
            QString dateTimeStr = dateTime.toString("yyyyMMddhhmmss");
            qDebug() << "dateTime:" << dateTimeStr;

            sprintf_s(pchStr, 128, "%sWO%s,", lockHexAddr.toLocal8Bit().data(),
                      dateTimeStr.toLocal8Bit().data());
            SPComm::getChksum(pchStr, chkSum);
            sprintf_s(pchStr, 128, "#%sWO%s,%02X\r", lockHexAddr.toLocal8Bit().data(),
                      dateTimeStr.toLocal8Bit().data(), *chkSum & 0xff);

            QByteArray readBuf;
            if(this->socketWriteAndRead(lockSocket,pchStr,readBuf))
            {
                if(!(readBuf.contains("OK")))
                {
                    QMessageBox::warning(NULL, QObject::tr("警告"),
                                         QObject::tr("编号为%1的锁具，\n开锁失败")
                                         .arg(lockCode),
                                         QMessageBox::Yes);
                }
                else
                {
                    //记录日志
                    if(ui->comboBox->currentText() == tr("管理员"))
                    {
                        MysqlOperation::insertRecord(ui->lineEdit->text(),
                                                    lockCode,
                                                     "NN2N4N6N",tr("监控中心开锁成功"));
                    }
                    else
                    {
                        MysqlOperation::insertRecord(ui->lineEdit->text(),
                                                     lockCode,
                                                     "NN2N4NN7",tr("监控中心开锁成功"));
                    }

                    QMessageBox::information(NULL, QObject::tr("提示"),
                                         QObject::tr("编号为%1的锁具，\n开锁成功")
                                         .arg(lockCode),
                                         QMessageBox::Yes);
                }
            }
        }
    }

    if(!atLeastOneSelFlag)
    {
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("请选择要打开的锁具"),
                                 QMessageBox::Yes);
    }

    delete[] pchStr;
    pchStr = NULL;
    delete chkSum;
    chkSum = NULL;

    ui->pushButton_34->setEnabled(true);
#else
    if(!m_startServer)
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxInfo(QObject::tr("请先启动服务器"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("请先启动服务器"),
                                 QMessageBox::Yes);
#endif
        return;
    }

    ui->pushButton_34->setEnabled(false);

    int flag = 1;
    for(int row=0; row < ui->tableWidget_2->rowCount(); row++)
    {
        if(((QCheckBox*)(ui->tableWidget_2->cellWidget(row, 0)))->isChecked())
        {
            flag = 0;
            ui->tableWidget_2->selectRow(row);
            //发送
            QString devCode = ui->tableWidget_2->item(row, 1)->text();
            if(this->setLockModeHand(devCode) == -1)
            {
#ifdef USE_SELF_MSGBOX
                myHelper::ShowMessageBoxError(QObject::tr("打开%1设备锁失败").arg(devCode));
#else
                QMessageBox::warning(NULL, QObject::tr("警告"),
                                     QObject::tr("打开%1设备锁失败").arg(devCode),
                                     QMessageBox::Yes);
#endif
                continue;
            }
            if(this->openLock(devCode) == -1)
            {
#ifdef USE_SELF_MSGBOX
                myHelper::ShowMessageBoxError(QObject::tr("打开%1设备锁失败").arg(devCode));
#else
                QMessageBox::warning(NULL, QObject::tr("警告"),
                                     QObject::tr("打开%1设备锁失败").arg(devCode),
                                     QMessageBox::Yes);
#endif
                continue;
            }

            bool openFlag =false;
            for(int n=0; n < 4; n++)
            {
                this->sleep(5000);

                for(int row=0; row < ui->tableWidget_2->rowCount(); row++)
                {
                    if(ui->tableWidget_2->item(row, 1)->text() == devCode)
                    {
                        if(ui->tableWidget_2->item(row, 6)->text().trimmed() == QObject::tr("开"))
                        {
                            if(ui->comboBox->currentText() == tr("操作员"))
                            {
                                MysqlOperation::insertRecord(ui->lineEdit->text(),devCode,"NN2N4N6N",tr("开锁成功"));
                            }
                            else
                            {
                                MysqlOperation::insertRecord(ui->lineEdit->text(),devCode,"NN2N4NN7",tr("开锁成功"));
                            }
                            openFlag = true;
                            break;
                        }
                    }
                }

                if(openFlag)
                {
                    break;
                }
            }

            this->setLockModeAuto(devCode);

        }//end if
    }//end for

    if(flag)
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxInfo(QObject::tr("请选择要打开的设备"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("请选择要打开的设备"),
                                 QMessageBox::Yes);
#endif
    }

    ui->pushButton_34->setEnabled(true);
#endif
}

void MainWindow::on_pushButton_78_clicked()
{
    this->updateRecordTableView(0);
}

void MainWindow::on_pushButton_83_clicked()
{
    this->updateRecordTableView(1);
}

void MainWindow::on_pushButton_84_clicked()
{
    this->updateRecordTableView(2);
}

void MainWindow::on_pushButton_87_clicked()
{
    this->updateRecordTableView(3);
}

void MainWindow::on_pushButton_88_clicked()
{
    this->updateRecordTableView(4);
}

void MainWindow::on_pushButton_39_clicked()
{
    this->updateRecordTableView(5);
}

void MainWindow::on_pushButton_92_clicked()
{
    this->updateRecordTableView(6);
}

void MainWindow::on_pushButton_25_clicked()
{
    this->updateRecordTableView(7);
}

void MainWindow::on_pushButton_65_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, QObject::tr("Excel file"),
                                                    qApp->applicationDirPath (),
                                                    QObject::tr("Excel Files (*.xls)"));
    if (fileName.isEmpty()||fileName.isNull())
    {
        qDebug() << QObject::tr("fileName is empty!");
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("保存文件名不允许为空！"));
#else
        QMessageBox::information(this, QObject::tr("提示"),
                                 QObject::tr("保存文件名不允许为空！"),
                                 QMessageBox::Yes);
#endif
        return;
    }

    qDebug() << QObject::tr("fileName is:%1").arg(fileName);

    ExportExcelObject obj(fileName, "mydata", ui->tableView);

    obj.addField(1, "姓名", "char(50)");
    obj.addField(2, "登录名", "char(50)");
    obj.addField(4, "所属权限组", "char(50)");
    obj.addField(5, "所属用户组", "char(50)");
    obj.addField(6, "联系方式", "char(50)");
    obj.addField(7, "权限状态", "char(50)");
    obj.addField(8, "权限起始日期", "char(50)");
    obj.addField(9, "权限截止日期", "char(50)");

    int retVal = obj.export2Excel();
    if( retVal > 0)
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxInfo(QObject::tr("%1条记录被导出").arg(retVal));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("%1条记录被导出").arg(retVal),
                                 QMessageBox::Yes);
#endif
    }
    else
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("导出excel文件失败"));
#else
        QMessageBox::warning(NULL, QObject::tr("警告"),
                                 QObject::tr("导出excel文件失败"),
                                 QMessageBox::Yes);
#endif
    }
}

void MainWindow::on_pushButton_67_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, QObject::tr("Excel file"),
                                                    qApp->applicationDirPath (),
                                                    QObject::tr("Excel Files (*.xls)"));
    if (fileName.isEmpty()||fileName.isNull())
    {
        qDebug() << QObject::tr("fileName is empty!");
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("保存文件名不允许为空！"));
#else
        QMessageBox::information(this, QObject::tr("提示"),
                                 QObject::tr("保存文件名不允许为空！"),
                                 QMessageBox::Yes);
#endif
        return;
    }

    qDebug() << QObject::tr("fileName is:%1").arg(fileName);

    ExportExcelObject obj(fileName, "mydata", ui->tableView_4);

    obj.addField(1, "姓名", "char(50)");
    obj.addField(2, "登录名", "char(50)");
    obj.addField(3, "所属权限组", "char(50)");
    obj.addField(4, "所属用户组", "char(50)");
    obj.addField(5, "联系方式", "char(50)");
    obj.addField(6, "权限状态", "char(50)");
    obj.addField(7, "权限起始日期", "char(50)");
    obj.addField(8, "权限截止日期", "char(50)");
    obj.addField(9, "下载授权票据时间", "char(50)");

    int retVal = obj.export2Excel();
    if( retVal > 0)
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxInfo(QObject::tr("%1条记录被导出").arg(retVal));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("%1条记录被导出").arg(retVal),
                                 QMessageBox::Yes);
#endif
    }
    else
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("导出excel文件失败"));
#else
        QMessageBox::warning(NULL, QObject::tr("警告"),
                                 QObject::tr("导出excel文件失败"),
                                 QMessageBox::Yes);
#endif
    }
}

void MainWindow::on_pushButton_64_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, QObject::tr("Excel file"),
                                                    qApp->applicationDirPath (),
                                                    QObject::tr("Excel Files (*.xls)"));
    if (fileName.isEmpty()||fileName.isNull())
    {
        qDebug() << QObject::tr("fileName is empty!");
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("保存文件名不允许为空！"));
#else
        QMessageBox::information(this, QObject::tr("提示"),
                                 QObject::tr("保存文件名不允许为空！"),
                                 QMessageBox::Yes);
#endif
        return;
    }

    qDebug() << QObject::tr("fileName is:%1").arg(fileName);

    ExportExcelObject obj(fileName, "mydata", ui->tableView_2);

    obj.addField(1, "姓名", "char(50)");
    obj.addField(2, "登录名", "char(50)");
    obj.addField(3, "密码", "char(50)");
    obj.addField(4, "所属权限组", "char(50)");
    obj.addField(5, "所属用户组", "char(50)");
    obj.addField(6, "联系方式", "char(50)");
    obj.addField(7, "权限状态", "char(50)");
    obj.addField(8, "权限起始日期", "char(50)");
    obj.addField(9, "权限截止日期", "char(50)");

    int retVal = obj.export2Excel();
    if( retVal > 0)
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxInfo(QObject::tr("%1条记录被导出").arg(retVal));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("%1条记录被导出").arg(retVal),
                                 QMessageBox::Yes);
#endif
    }
    else
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("导出excel文件失败"));
#else
        QMessageBox::warning(NULL, QObject::tr("警告"),
                                 QObject::tr("导出excel文件失败"),
                                 QMessageBox::Yes);
#endif
    }
}

void MainWindow::on_pushButton_68_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, QObject::tr("Excel file"),
                                                    qApp->applicationDirPath (),
                                                    QObject::tr("Excel Files (*.xls)"));
    if (fileName.isEmpty()||fileName.isNull())
    {
        qDebug() << QObject::tr("fileName is empty!");
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("保存文件名不允许为空！"));
#else
        QMessageBox::information(this, QObject::tr("提示"),
                                 QObject::tr("保存文件名不允许为空！"),
                                 QMessageBox::Yes);
#endif
        return;
    }

    qDebug() << QObject::tr("fileName is:%1").arg(fileName);

    ExportExcelObject obj(fileName, "mydata", ui->tableView_5);

    obj.addField(1, "锁编号", "char(50)");
    obj.addField(2, "锁名称", "char(50)");
    obj.addField(3, "所属用户组", "char(50)");
    obj.addField(4, "地理位置", "char(50)");
    obj.addField(5, "经度", "char(50)");
    obj.addField(6, "纬度", "char(50)");
    obj.addField(7, "fsuIP地址", "char(50)");
    //obj.addField(8, "锁用户", "char(50)");
    obj.addField(9, "服役时间", "char(50)");
    //obj.addField(10, "锁ID", "char(50)");

    int retVal = obj.export2Excel();
    if( retVal > 0)
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxInfo(QObject::tr("%1条记录被导出").arg(retVal));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("%1条记录被导出").arg(retVal),
                                 QMessageBox::Yes);
#endif
    }
    else
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("导出excel文件失败"));
#else
        QMessageBox::warning(NULL, QObject::tr("警告"),
                                 QObject::tr("导出excel文件失败"),
                                 QMessageBox::Yes);
#endif
    }
}

void MainWindow::on_pushButton_63_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, QObject::tr("Excel file"),
                                                    qApp->applicationDirPath (),
                                                    QObject::tr("Excel Files (*.xls)"));
    if (fileName.isEmpty()||fileName.isNull())
    {
        qDebug() << QObject::tr("fileName is empty!");
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("保存文件名不允许为空！"));
#else
        QMessageBox::information(this, QObject::tr("提示"),
                                 QObject::tr("保存文件名不允许为空！"),
                                 QMessageBox::Yes);
#endif
        return;
    }

    qDebug() << QObject::tr("fileName is:%1").arg(fileName);

    ExportExcelObject obj(fileName, "mydata", ui->tableView_7);

    obj.addField(1, "钥匙编号", "char(50)");
    obj.addField(2, "钥匙名称", "char(50)");
    obj.addField(3, "所属用户组", "char(50)");
    //obj.addField(4, "A级密钥", "char(50)");
    //obj.addField(5, "B级密钥", "char(50)");
    obj.addField(6, "权限状态", "char(50)");
    obj.addField(7, "权限掩码", "char(50)");
    obj.addField(8, "权限起始日期", "char(50)");
    obj.addField(9, "权限截止日期", "char(50)");
    //obj.addField(10, "钥匙ID", "char(50)");

    int retVal = obj.export2Excel();
    if( retVal > 0)
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxInfo(QObject::tr("%1条记录被导出").arg(retVal));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("%1条记录被导出").arg(retVal),
                                 QMessageBox::Yes);
#endif
    }
    else
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("导出excel文件失败"));
#else
        QMessageBox::warning(NULL, QObject::tr("警告"),
                                 QObject::tr("导出excel文件失败"),
                                 QMessageBox::Yes);
#endif
    }
}

void MainWindow::on_pushButton_90_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, QObject::tr("Excel file"),
                                                    qApp->applicationDirPath (),
                                                    QObject::tr("Excel Files (*.xls)"));
    if (fileName.isEmpty()||fileName.isNull())
    {
        qDebug() << QObject::tr("fileName is empty!");
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("保存文件名不允许为空！"));
#else
        QMessageBox::information(this, QObject::tr("提示"),
                                 QObject::tr("保存文件名不允许为空！"),
                                 QMessageBox::Yes);
#endif
        return;
    }

    qDebug() << QObject::tr("fileName is:%1").arg(fileName);

    ExportExcelObject obj(fileName, "mydata", ui->tableView_9);

    obj.addField(1, "姓名", "char(50)");
    obj.addField(2, "设备编号", "char(50)");
    //obj.addField(3, "日志类型", "char(50)");
    obj.addField(4, "事件内容", "char(50)");
    obj.addField(5, "发生时间", "char(50)");

    int retVal = obj.export2Excel();
    if( retVal > 0)
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxInfo(QObject::tr("%1条记录被导出").arg(retVal));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("%1条记录被导出").arg(retVal),
                                 QMessageBox::Yes);
#endif
    }
    else
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("导出excel文件失败"));
#else
        QMessageBox::warning(NULL, QObject::tr("警告"),
                                 QObject::tr("导出excel文件失败"),
                                 QMessageBox::Yes);
#endif
    }
}

void MainWindow::on_pushButton_27_clicked()
{
    QItemSelectionModel *selections = ui->tableView_9->selectionModel();
    QModelIndexList selected = selections->selectedIndexes();

    bool flag = false;
    foreach (QModelIndex index, selected)
    {
        if(!flag)
        {
#ifdef USE_SELF_MSGBOX
            int ok = myHelper::ShowMessageBoxQuesion(QObject::tr("是否删除选中的记录？"));
            if(ok != 1)
#else
            int ok = QMessageBox::warning(NULL, QObject::tr("警告"),
                                          QObject::tr("是否删除选中的记录？"),
                                          QMessageBox::Yes | QMessageBox::No,
                                          QMessageBox::Yes);
            if(ok == QMessageBox::No)
#endif
            {
                return;
            }
        }

        flag = true;
        m_recordModel.removeRow(index.row());  //删除该行
    }

    if(!flag)
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxInfo(QObject::tr("请选择要删除的记录"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("请选择要删除的记录"),
                                 QMessageBox::Yes);
#endif
        return;
    }

    if(m_recordModel.submitAll())
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxInfo(QObject::tr("删除成功"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("删除成功"),
                                 QMessageBox::Yes);
#endif

    }
    else
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("删除失败"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("删除失败"),
                                 QMessageBox::Yes);
#endif
    }

}

void MainWindow::on_pushButton_85_clicked()
{
    this->updateRecordTableView(10);
}

void MainWindow::on_pushButton_89_clicked()
{
    m_recordModel.setTable("recordTable");
    m_recordModel.setEditStrategy(QSqlTableModel::OnManualSubmit);

    //设置过滤器
    if(ui->radioButton_13->isChecked())
    {
        m_recordModel.setFilter(QString("callName='%1'").arg(ui->lineEdit_30->text().trimmed()));
    }

    if(ui->radioButton_14->isChecked())
    {
        m_recordModel.setFilter(QString("devCode='%1'").arg(ui->lineEdit_4->text().trimmed()));
    }

    if(ui->radioButton_15->isChecked())
    {
        QDateTime time = ui->dateTimeEdit_4->dateTime();
        qDebug() << tr("按时间查询:") << time.toString("yyyy-MM-dd hh:mm:ss");
        m_recordModel.setFilter(tr("loadTime >= '%1'")
                                     .arg(time.toString("yyyy-MM-dd hh:mm:ss")));
    }

    m_recordModel.select();
    m_recordModel.setHeaderData(1, Qt::Horizontal, QObject::tr("姓名"));
    m_recordModel.setHeaderData(2, Qt::Horizontal, QObject::tr("设备编号"));
    m_recordModel.setHeaderData(4, Qt::Horizontal, QObject::tr("事件内容"));
    m_recordModel.setHeaderData(5, Qt::Horizontal, QObject::tr("发生时间"));

    ui->tableView_9->setModel(&m_recordModel);
    ui->tableView_9->hideColumn(0);
    ui->tableView_9->hideColumn(3);

    ui->tableView_9->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}
