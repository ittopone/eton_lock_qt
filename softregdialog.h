#ifndef SOFTREGDIALOG_H
#define SOFTREGDIALOG_H

#include <QDialog>

namespace Ui {
class SoftRegDialog;
}

class SoftRegDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SoftRegDialog(QWidget *parent = 0);
    ~SoftRegDialog();

private:
    Ui::SoftRegDialog *ui;
};

#endif // SOFTREGDIALOG_H
