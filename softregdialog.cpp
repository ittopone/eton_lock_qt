#include "softregdialog.h"
#include "ui_softregdialog.h"

SoftRegDialog::SoftRegDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SoftRegDialog)
{
    ui->setupUi(this);
}

SoftRegDialog::~SoftRegDialog()
{
    delete ui;
}
