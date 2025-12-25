#include "registerdialog.h"
#include "ui_registerdialog.h"
#include "global.h"

RegisterDialog::RegisterDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RegisterDialog)
{
    ui->setupUi(this);
    ui->pwd_edit->setEchoMode(QLineEdit::Password);
    ui->confirm_edit->setEchoMode(QLineEdit::Password);
    ui->err_tip->setProperty("state", "normal");
    repolish(ui->err_tip);
}

RegisterDialog::~RegisterDialog()
{
    delete ui;
}

void RegisterDialog::on_verify_button_clicked()
{
    auto email = ui->email_edit->text();
    //匹配邮箱的正确的正则表达式
     QRegularExpression regex(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)");
    bool isMatch = regex.match(email).hasMatch();
     if (isMatch) {
         //发送验证码
         showTip(tr("验证码已发送！！！"), true);
     } else {
         //显示错误提示
         showTip(tr("邮箱输入错误！！！"), false);
     }
}

void RegisterDialog::showTip(QString str, bool state)
{
    ui->err_tip->setText(str);
    if (state) {
        ui->err_tip->setProperty("state","normal");
    } else {
        ui->err_tip->setProperty("state","err");
    }
    repolish(ui->err_tip);
}

