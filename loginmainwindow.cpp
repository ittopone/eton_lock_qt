#include "loginmainwindow.h"
#include "ui_loginmainwindow.h"
#include "selfdefine.h"
#include "mainwindow.h"
#include "mysqloperation.h"
#include <QMessageBox>
#include <QFile>
#include <QSqlTableModel>
#include <QHostInfo>
#include <QCryptographicHash>

LoginMainWindow::LoginMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::LoginMainWindow)
{
    ui->setupUi(this);

    //关乎界面
    myHelper::FormInCenter(this);
    this->InitStyle();

    //初始化成员变量
    m_isRegister = false;

    connect(ui->listWidget, SIGNAL(currentRowChanged(int)), ui->stackedWidget, SLOT(setCurrentIndex(int)));
    ui->listWidget->setCurrentRow(0);

    //ui->textBrowser_2->setTextColor();

    //读取数据库配置信息
    QFile file("DatabaseConfig.ini");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        for(int lineNum = 0; !in.atEnd(); lineNum++)
        {
            QString line = in.readLine();
            qDebug() << line;
            switch(lineNum)
            {
            case 0:
                ui->lineEdit_3->setText(line);
                MysqlOperation::MysqlAddr = line;
                break;
            case 1:
                ui->lineEdit_4->setText(line);
                MysqlOperation::MysqlDatabaseName = line;
                break;
            case 2:
                ui->spinBox->setValue(line.toInt(NULL, 10));
                MysqlOperation::MysqlPort = line;
                break;
            case 3:
                ui->lineEdit_6->setText(line);
                MysqlOperation::MysqlUserName = line;
            case 4:
                ui->lineEdit_7->setText(line);
                MysqlOperation::MysqlPassword = line;
                break;
            default:
                break;
            }
        }

        file.close();
    }

    //读注册码
    QFile file_2("registerCode.ini");
    if (file_2.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file_2);
        for(int lineNum = 0; !in.atEnd(); lineNum++)
        {
            QString line = in.readLine();
            qDebug() << line;
            switch(lineNum)
            {
            case 0:
            {
                ui->lineEdit_9->setText(line);

                QString localHostName = QHostInfo::localHostName();
                localHostName.append("eton");
                QByteArray byte_array;
                byte_array.append(localHostName);
                qDebug() << tr("about to code to md5:") << byte_array;
                QByteArray hash_byte_array = QCryptographicHash::hash(byte_array, QCryptographicHash::Md5);
                QString md5 = hash_byte_array.toHex();

                qDebug() << tr("md5:") << md5;

                QString registerCode = ui->lineEdit_9->text().trimmed();
                QStringList strList = registerCode.split("-");
                QString inputMD5;
                for(int i=0; i < strList.size(); i++)
                {
                    inputMD5.append(strList.at(i));
                }

                qDebug() << tr("input md5:") << inputMD5;

                if(inputMD5.isEmpty()||inputMD5.isNull())
                {
                    ui->label_10->setText(tr("软件未注册，使用期限为 0 天。"));
                    m_isRegister = false;
                }

                if(inputMD5 != md5)
                {
                    ui->label_10->setText(tr("软件未注册，使用期限为 0 天。"));
                    m_isRegister = false;
                }
                else
                {
                    m_isRegister = true;
                    ui->label_10->setText(tr("软件已注册，使用期限为永久。"));
                    ui->lineEdit_9->setEnabled(false);
                    ui->pushButton_5->setEnabled(false);
                }
            }

                break;
            default:
                break;
            }
        }

        file_2.close();
    }

}

LoginMainWindow::~LoginMainWindow()
{
    delete ui;
}

void LoginMainWindow::InitStyle()
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

bool LoginMainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonDblClick) {
        this->on_btnMenu_Max_clicked();
        return true;
    }
    return QObject::eventFilter(obj, event);
}

void LoginMainWindow::mouseMoveEvent(QMouseEvent *e)
{
    if (mousePressed && (e->buttons() && Qt::LeftButton) && !max) {
        this->move(e->globalPos() - mousePoint);
        e->accept();
    }
}

void LoginMainWindow::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        mousePressed = true;
        mousePoint = e->globalPos() - this->pos();
        e->accept();
    }
}

void LoginMainWindow::mouseReleaseEvent(QMouseEvent *)
{
    mousePressed = false;
}

void LoginMainWindow::on_btnMenu_Close_clicked()
{
    qApp->exit();
}

void LoginMainWindow::on_btnMenu_Max_clicked()
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

void LoginMainWindow::on_btnMenu_Min_clicked()
{
    this->showMinimized();
}

void LoginMainWindow::on_pushButton_4_clicked()
{
    QString superName = ui->lineEdit_8->text().trimmed();
    QString superPswd = ui->lineEdit_11->text().trimmed();
    if(superName.trimmed() != "etonYujunwu")
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("超级用户名错误！"));
#else
        QMessageBox::warning(NULL, QObject::tr("警告"),
                                 QObject::tr("超级用户名错误！"),
                                 QMessageBox::Yes);
#endif
        return;
    }
    if(superPswd.trimmed() != "eton123456")
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("超级密钥错误！"));
#else
        QMessageBox::warning(NULL, QObject::tr("警告"),
                                 QObject::tr("超级密钥错误！"),
                                 QMessageBox::Yes);
#endif
        return;
    }

    if(!MysqlOperation::m_dbIsconnect)
    {
        //连接数据库
        if(!MysqlOperation::openMysqlDatabase())
        {
#ifdef USE_SELF_MSGBOX
            myHelper::ShowMessageBoxError(QObject::tr("打开数据库失败"));
#else
            QMessageBox::warning(NULL, QObject::tr("警告"),
                                     QObject::tr("打开数据库失败"),
                                     QMessageBox::Yes);
#endif
            return;
        }
    }

    QSqlTableModel *userModel = new QSqlTableModel;
    userModel->setTable("adminTable");
    if(!(userModel->select()))
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("某个数据库表未创建！"));
#else
        QMessageBox::warning(NULL, QObject::tr("警告"),
                                 QObject::tr("某个数据库表未创建！"),
                                 QMessageBox::Yes);
#endif
        return;
    }
    userModel->setHeaderData(1, Qt::Horizontal, QObject::tr("姓名"));
    userModel->setHeaderData(2, Qt::Horizontal, QObject::tr("登录名"));
    userModel->setHeaderData(3, Qt::Horizontal, QObject::tr("密码"));
    ui->tableView->setModel(userModel);
    ui->tableView->hideColumn(0);
    ui->tableView->hideColumn(4);
    ui->tableView->hideColumn(5);
    ui->tableView->hideColumn(6);
    ui->tableView->hideColumn(7);
    ui->tableView->hideColumn(8);
    ui->tableView->hideColumn(9);

    ui->stackedWidget->setCurrentIndex(5);
}

void LoginMainWindow::on_pushButton_3_clicked()
{
     ui->listWidget->setCurrentRow(0);
}

void LoginMainWindow::on_pushButton_clicked()
{
    if(!m_isRegister)
    {
        myHelper::ShowMessageBoxError(tr("请先完成软件的注册"));
        return;
    }

    QString userName = ui->lineEdit->text();
    QString password = ui->lineEdit_2->text();
    if(userName.isEmpty()||userName.isNull())
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("登录名不允许为空"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("登录名不允许为空"),
                                 QMessageBox::Yes);
#endif
        return;
    }
    if(password.isEmpty()||password.isNull())
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("密码不允许为空"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("密码不允许为空"),
                                 QMessageBox::Yes);
#endif
        return;
    }

    if(!MysqlOperation::m_dbIsconnect)
    {
        //连接数据库
        if(!MysqlOperation::openMysqlDatabase())
        {
#ifdef USE_SELF_MSGBOX
            myHelper::ShowMessageBoxError(QObject::tr("打开数据库失败"));
#else
            QMessageBox::warning(NULL, QObject::tr("警告"),
                                     QObject::tr("打开数据库失败"),
                                     QMessageBox::Yes);
#endif
            return;
        }
    }

    //创建用户数据库表
    if(!MysqlOperation::createAdminTable())
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("创建admin数据库表失败"));
#else
        QMessageBox::warning(NULL, QObject::tr("警告"),
                                 QObject::tr("创建admin数据库表失败"),
                                 QMessageBox::Yes);
#endif

        MysqlOperation::closeMysqlDatabase();

        return;
    }
    //创建锁数据库表
    if(!MysqlOperation::createLockTable())
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("创建lock数据库表失败"));
#else
        QMessageBox::warning(NULL, QObject::tr("警告"),
                             QObject::tr("创建lock数据库表失败"),
                             QMessageBox::Yes);
#endif

         MysqlOperation::closeMysqlDatabase();

        return;
    }
    //创建钥匙数据库表
    if(!MysqlOperation::createKeyTable())
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("创建key数据库表失败"));
#else
        QMessageBox::warning(NULL, QObject::tr("警告"),
                             QObject::tr("创建key数据库表失败"),
                             QMessageBox::Yes);
#endif

         MysqlOperation::closeMysqlDatabase();

        return;
    }
    //创建日志数据库表
    if(!MysqlOperation::createRecordTable())
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("创建record数据库表失败"));
#else
        QMessageBox::warning(NULL, QObject::tr("警告"),
                             QObject::tr("创建record数据库表失败"),
                             QMessageBox::Yes);
#endif

         MysqlOperation::closeMysqlDatabase();

        return;
    }
    //所属用户组数据表
    if(!MysqlOperation::createGroupTable())
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("创建group数据库表失败"));
#else
        QMessageBox::warning(NULL, QObject::tr("警告"),
                             QObject::tr("创建group数据库表失败"),
                             QMessageBox::Yes);
#endif

        MysqlOperation::closeMysqlDatabase();

        return;
    }
    if(!MysqlOperation::createEmpowerTable())
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("创建empower数据库表失败"));
#else
        QMessageBox::warning(NULL, QObject::tr("警告"),
                             QObject::tr("创建empower数据库表失败"),
                             QMessageBox::Yes);
#endif

        MysqlOperation::closeMysqlDatabase();

        return;
    }

#if 1
    QVector<QString> adminInfoVector;
    if(MysqlOperation::getAdminInfoByLogName(userName,adminInfoVector))
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

    int retIsIndb = MysqlOperation::isAdminInDatabase(userName,password);
    if(retIsIndb == 0)
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("该用户不存在"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("该用户不存在"),
                                 QMessageBox::Yes);
#endif

        MysqlOperation::closeMysqlDatabase();

        return;
    }
    if(retIsIndb == -1)
    {
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("查询数据库失败"));
#else
        QMessageBox::warning(NULL, QObject::tr("警告"),
                                 QObject::tr("查询数据库失败"),
                                 QMessageBox::Yes);
#endif

        MysqlOperation::closeMysqlDatabase();

        return;
    }

    int retIsEmpower = MysqlOperation::isAdminEmpower(userName,password);
    switch(retIsEmpower)
    {
    case 0:
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("该用户未授权"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("该用户未授权"),
                                 QMessageBox::Yes);
#endif
        MysqlOperation::closeMysqlDatabase();
        return;
    case -1:
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("查询数据库失败"));
#else
        QMessageBox::warning(NULL, QObject::tr("警告"),
                                 QObject::tr("查询数据库失败"),
                                 QMessageBox::Yes);
#endif
        MysqlOperation::closeMysqlDatabase();
        return;
    case -2:
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("该用户权限过期"));
#else
        QMessageBox::warning(NULL, QObject::tr("警告"),
                                 QObject::tr("该用户权限过期"),
                                 QMessageBox::Yes);
#endif
        MysqlOperation::closeMysqlDatabase();
        return;
    case -3:
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("该用户权限未到"));
#else
        QMessageBox::warning(NULL, QObject::tr("警告"),
                                 QObject::tr("该用户权限未到"),
                                 QMessageBox::Yes);
#endif
        MysqlOperation::closeMysqlDatabase();
        return;
    case -4:
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxError(QObject::tr("该用户已被拉入黑名单"));
#else
        QMessageBox::warning(NULL, QObject::tr("警告"),
                                 QObject::tr("该用户已被拉入黑名单"),
                                 QMessageBox::Yes);
#endif
        MysqlOperation::closeMysqlDatabase();
        return;
    default:
        break;
    }
#endif

    MainWindow *mainWindow = new MainWindow(0, userName);
    mainWindow->show();
    this->close();
}

void LoginMainWindow::on_pushButton_5_clicked()
{
    QString localHostName = QHostInfo::localHostName();
    localHostName.append("eton");
    QByteArray byte_array;
    byte_array.append(localHostName);
    qDebug() << tr("about to code to md5:") << byte_array;
    QByteArray hash_byte_array = QCryptographicHash::hash(byte_array, QCryptographicHash::Md5);
    QString md5 = hash_byte_array.toHex();

    qDebug() << tr("md5:") << md5;

    QString registerCode = ui->lineEdit_9->text().trimmed();
    QStringList strList = registerCode.split("-");
    QString inputMD5;
    for(int i=0; i < strList.size(); i++)
    {
        inputMD5.append(strList.at(i));
    }

    qDebug() << tr("input md5:") << inputMD5;

    if(inputMD5.isEmpty()||inputMD5.isNull())
    {
        ui->label_10->setText(tr("软件未注册，使用期限为 0 天。"));
        myHelper::ShowMessageBoxError(tr("注册码不能为空！"));
        m_isRegister = false;
        return;
    }

    if(inputMD5 != md5)
    {
        ui->label_10->setText(tr("软件未注册，使用期限为 0 天。"));
        myHelper::ShowMessageBoxError(tr("注册码错误！"));
        m_isRegister = false;
        return;
    }

    m_isRegister = true;

    myHelper::ShowMessageBoxInfo(tr("注册成功！"));

    QFile file("registerCode.ini");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&file);
        out << ui->lineEdit_9->text().trimmed() << "\n";
        file.close();
    }

    ui->label_10->setText(tr("软件已注册，使用期限为永久。"));
    ui->lineEdit_9->setEnabled(false);
    ui->pushButton_5->setEnabled(false);

}

void LoginMainWindow::on_pushButton_2_clicked()
{
    QFile file("DatabaseConfig.ini");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&file);
        out << ui->lineEdit_3->text().trimmed() << "\n";
        out << ui->lineEdit_4->text().trimmed() << "\n";
        out << ui->spinBox->text().trimmed() << "\n";
        out << ui->lineEdit_6->text().trimmed() << "\n";
        out << ui->lineEdit_7->text().trimmed() << "\n";
        file.close();

        MysqlOperation::MysqlAddr = ui->lineEdit_3->text().trimmed();
        MysqlOperation::MysqlDatabaseName = ui->lineEdit_4->text().trimmed();
        MysqlOperation::MysqlPort = ui->spinBox->text().trimmed();
        MysqlOperation::MysqlUserName = ui->lineEdit_6->text().trimmed();
        MysqlOperation::MysqlPassword = ui->lineEdit_7->text().trimmed();
#ifdef USE_SELF_MSGBOX
        myHelper::ShowMessageBoxInfo(QObject::tr("保存设置成功"));
#else
        QMessageBox::information(NULL, QObject::tr("提示"),
                                 QObject::tr("保存设置成功"),
                                 QMessageBox::Yes);
#endif
    }
}

void LoginMainWindow::on_pushButton_7_clicked()
{
    this->close();
}

void LoginMainWindow::on_pushButton_8_clicked()
{
#ifdef USE_SELF_MSGBOX
    myHelper::ShowMessageBoxInfo(QObject::tr("该软件已经是最新版"));
#else
    QMessageBox::information(NULL, QObject::tr("提示"),
                             QObject::tr("该软件已经是最新版"),
                             QMessageBox::Yes);
#endif
}
