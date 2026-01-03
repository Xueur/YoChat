#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    login_dlg = new LoginDialog(this);
    this->setCentralWidget(login_dlg);
    // login_dlg->show();
    //创建和注册界面链接
    connect(login_dlg, &LoginDialog::switchRegister, this, &MainWindow::slotSwitchReg);
    reg_dlg = new RegisterDialog(this);
    //这里用了Dialog，实际上嵌入用Widget最好
    login_dlg->setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint);
    reg_dlg->setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::slotSwitchReg()
{
    setCentralWidget(reg_dlg);
    login_dlg->hide();
    // reg_dlg->show();
}
