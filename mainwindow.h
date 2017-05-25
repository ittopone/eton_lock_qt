#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlTableModel>
#include <QHostAddress>
#include <QTcpServer>
#include <QLabel>
#include <QMap>

#include "iconhelper.h"
#include "myhelper.h"
#include <QSystemTrayIcon>
#include <QProgressBar>
#include "mycontroller.h"
#include "myworkerthread.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0,QString logName = "");

    ~MainWindow();

protected:
    virtual void closeEvent(QCloseEvent * event);

public slots:
    QString jsInvokQt(QString,QString);

    void addObjectToJs();

private slots:
    void myslot_checkLockOpenStatusTimeOut();
#if 0
    void myslot_timeOutExec();
#endif

    void myslot_markLockOnBaiduMap(bool);

    //托盘相关
    void myslot_trayIconActivated(QSystemTrayIcon::ActivationReason);

    void myslot_readMessage();

    void myslot_clientDisconnect();

    void myslot_updateStatusBarMaxCount(int);

    void myslot_updateStatusBarPort(int);

    void myslot_processConnection();

    void autoScroll();

    void setCurrentStackPageSlot(int);

    void on_pushButton_6_clicked();

    void on_pushButton_45_clicked();

    void on_pushButton_47_clicked();

    void on_pushButton_46_clicked();

    void on_pushButton_48_clicked();

    void on_pushButton_49_clicked();

    void on_pushButton_50_clicked();  

    void on_pushButton_55_clicked();

    void on_pushButton_56_clicked();

    void on_pushButton_30_clicked();

    void on_pushButton_29_clicked();

    void on_pushButton_31_clicked();

    void on_pushButton_32_clicked();

    void on_pushButton_35_clicked();

    void on_pushButton_69_clicked();

    void on_pushButton_33_clicked();

    void on_pushButton_95_clicked();

    void on_pushButton_38_clicked();

    void on_pushButton_96_clicked();

    void on_pushButton_10_clicked();

    void on_pushButton_9_clicked();

    void on_pushButton_7_clicked();

    void on_pushButton_11_clicked();

    void on_pushButton_20_clicked();

    void on_pushButton_22_clicked();

    void on_pushButton_17_clicked();

    void on_pushButton_24_clicked();

    void on_pushButton_23_clicked();

    void on_pushButton_28_clicked();

    void on_pushButton_18_clicked();

    void on_pushButton_44_clicked();

    void on_pushButton_41_clicked();

    void on_pushButton_40_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_8_clicked();

    void on_pushButton_26_clicked();

    void on_pushButton_13_clicked();

    void on_pushButton_19_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_70_clicked();

    void on_pushButton_12_clicked();

    void on_pushButton_57_clicked();

    void on_pushButton_14_clicked();

    void on_pushButton_15_clicked();

    void on_pushButton_59_clicked();

    void on_pushButton_58_clicked();

    void on_pushButton_60_clicked();

    void on_pushButton_61_clicked();

    void on_pushButton_66_clicked();

    void on_pushButton_21_clicked();

    void on_pushButton_72_clicked();

    void on_pushButton_75_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_clicked();

    void on_pushButton_37_clicked();

    void on_pushButton_36_clicked();

    void on_pushButton_16_clicked();

    void on_pushButton_81_clicked();

    void on_pushButton_86_clicked();

    void on_pushButton_34_clicked();

    void on_pushButton_78_clicked();

    void on_pushButton_83_clicked();

    void on_pushButton_84_clicked();

    void on_pushButton_87_clicked();

    void on_pushButton_88_clicked();

    void on_pushButton_39_clicked();

    void on_pushButton_92_clicked();

    void on_pushButton_25_clicked();

    void on_pushButton_65_clicked();

    void on_pushButton_67_clicked();

    void on_pushButton_64_clicked();

    void on_pushButton_68_clicked();

    void on_pushButton_63_clicked();

    void on_pushButton_90_clicked();

    void on_btnMenu_Close_clicked();

    void on_btnMenu_Max_clicked();

    void on_btnMenu_Min_clicked();

    void on_pushButton_27_clicked();

    void on_pushButton_85_clicked();

    void on_pushButton_89_clicked();

private:
    //DO开锁操作
    int getCRC16(char *, int, char *);

    void sleep(unsigned int);

    int setLockModeHand(QString);

    int openLock(QString);

    int setLockModeAuto(QString);

    int getLockOpenStatus(QString);


    void updateRecordTableView(int);

    void updateAdminTableView();

    void updateGroupTableView();

    void updateLockTableView();

    void updateLockTableWidget();

    void updateKeyTableView();

    void updateEmpowerTableView();

    void updateLoadEmpowerTableView();

    void updateGroupComboBox();

    void updatePortNameComboBox();

    void updateAdminInfo();

    QHostAddress getLocalHostIP();

    //初始化状态栏
    void initStatusBar();

    bool getKeyAddress(QString&);

    QString hex82IP(QString);

    QString IP28hex(QString);

    int getKeyEmpowerStatus(QString);

    //启动服务器
    bool startServer();

    //锁操作
    bool getLockAddress(QTcpSocket *,QString &);
    bool socketWriteAndRead(QTcpSocket *, char *, QByteArray&);

protected:
    bool eventFilter(QObject *obj, QEvent *event);
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *);

private:
    Ui::MainWindow *ui;

    //状态栏
    QLabel *serIPLabel;

    QLabel *serPortLabel;

    QLabel *serStatusLabel;

    QLabel *currentNumLabel;

    QLabel *maxNumLabel;

    QProgressBar *m_progressBar;

    QString m_logName;

    //服务器参数
    bool m_startServer;

    int m_currentConetCount;

    QTcpServer *m_listenSocket;

    QMap<QTcpSocket*, QString> m_mapNetIsKey;

    QMap<QString, QTcpSocket*> m_mapDevCodeIsKey;

    QSqlTableModel m_adminModel;

    QSqlTableModel m_groupModel;

    QSqlTableModel m_lockModel;

    QSqlTableModel m_keyModel;

    QSqlTableModel m_empowerModel;

    QSqlTableModel m_loadEmpowerModel;

    QSqlTableModel m_recordModel;
    int m_selectFilterNum;

    //使用css模版
    QPoint mousePoint;
    bool mousePressed;
    bool max;
    QRect location;
    void InitStyle();

    //开启线程
#if 0
    MyController * m_controller;
#else
    MyWorkerThread *m_workerThread;
#endif
};

#endif // MAINWINDOW_H
