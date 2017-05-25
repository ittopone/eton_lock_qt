#ifndef LOGINMAINWINDOW_H
#define LOGINMAINWINDOW_H

#include <QMainWindow>

#include "iconhelper.h"
#include "myhelper.h"

namespace Ui {
class LoginMainWindow;
}

class LoginMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit LoginMainWindow(QWidget *parent = 0);
    ~LoginMainWindow();

protected:
    bool eventFilter(QObject *obj, QEvent *event);
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *);

private slots:
    void on_btnMenu_Close_clicked();

    void on_btnMenu_Max_clicked();

    void on_btnMenu_Min_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_7_clicked();

    void on_pushButton_8_clicked();

private:
    Ui::LoginMainWindow *ui;

    bool m_isRegister;

    QPoint mousePoint;
    bool mousePressed;
    bool max;
    QRect location;

    void InitStyle();
};

#endif // LOGINMAINWINDOW_H
