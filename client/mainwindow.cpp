#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <MainWindow.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    login_dlg = new LoginDialog(this);
    login_dlg->setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint);
    this->setCentralWidget(login_dlg);
    //连接登录界面注册信号
    connect(login_dlg, &LoginDialog::switchRegister, this, &MainWindow::SlotSwitchReg);
    //连接登录界面重置密码信号
    connect(login_dlg, &LoginDialog::switchReset, this, &MainWindow::SlotSwitchReset);
    //连接登录界面登录结束信号
    connect(login_dlg, &LoginDialog::switchChat, this, &MainWindow::SlotSwitchChat);
    //测试用
    emit login_dlg->switchChat();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::SlotSwitchReg()
{
    reg_dlg = new RegisterDialog(this);
    //连接注册界面返回登录信号
    connect(reg_dlg, &RegisterDialog::sigSwitchLogin, this, &MainWindow::SlotSwitchLogin);
    reg_dlg->hide();
    reg_dlg->setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint);
    setCentralWidget(reg_dlg);
    login_dlg->hide();
    reg_dlg->show();
}

void MainWindow::SlotSwitchLogin()
{
    login_dlg = new LoginDialog(this);
    login_dlg->setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint);
    setCentralWidget(login_dlg);
    reg_dlg->hide();
    login_dlg->show();
}

void MainWindow::SlotSwitchReset()
{
    reset_dlg = new ResetDialog(this);
    //连接重置密码界面返回登录信号
    connect(reset_dlg, &ResetDialog::switchLogin, this, &MainWindow::SlotSwitchLogin2);
    reset_dlg->setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint);
    setCentralWidget(reset_dlg);
    login_dlg->hide();
    reset_dlg->show();
}

void MainWindow::SlotSwitchLogin2()
{
    login_dlg = new LoginDialog(this);
    login_dlg->setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint);
    setCentralWidget(login_dlg);
    reset_dlg->hide();
    login_dlg->show();
}

void MainWindow::SlotSwitchChat()
{
    chat_dlg = new ChatDialog(this);
    chat_dlg->setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint);
    setCentralWidget(chat_dlg);
    login_dlg->hide();
    chat_dlg->show();
    this->setMinimumSize(QSize(1000, 800));
    this->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
}
