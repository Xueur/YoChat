#include "findfaileddialog.h"
#include "ui_findfaileddialog.h"
#include <QDebug>

FindFailedDialog::FindFailedDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FindFailedDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    this->setObjectName("FindFailedDialog");
    ui->fail_sure_btn->SetState("normal", "hover", "press");
    this->setModal(true);
}

FindFailedDialog::~FindFailedDialog()
{
    qDebug() << "FinfFailedDialog Destruct";
    delete ui;
}



void FindFailedDialog::on_fail_sure_btn_clicked()
{
    this->hide();
}

