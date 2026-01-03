#include "registerdialog.h"
#include "ui_registerdialog.h"
#include "global.h"
#include "httpmgr.h"

RegisterDialog::RegisterDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RegisterDialog)
{
    ui->setupUi(this);
    ui->pwd_edit->setEchoMode(QLineEdit::Password);
    ui->confirm_edit->setEchoMode(QLineEdit::Password);
    ui->err_tip->setProperty("state", "normal");
    repolish(ui->err_tip);
    connect(HttpMgr::getInstance().get(), &HttpMgr::sig_reg_mod_finish, this, &RegisterDialog::slot_reg_mod_finish);
    initHttpHandlers();
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
         QJsonObject json_obj;
         json_obj["email"] = email;
         HttpMgr::getInstance()->PostHttpReq(QUrl(gate_url_prefix+"/get_varifycode"),
                                             json_obj, ReqId::ID_GET_VARIFY_CODE,Modules::REGISTERMOD);
     } else {
         //显示错误提示
         showTip(tr("邮箱输入错误！！！"), false);
     }
}

void RegisterDialog::slot_reg_mod_finish(ReqId id, QString res, ErrorCodes err)
{
    if (err != ErrorCodes::SUCCESS) {
        showTip(tr("网络请求错误"), false);
        return;
    }
    //解析JSON 字符串， res 转化为QByteArray
    QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());
    if (jsonDoc.isNull()) {
        showTip(tr("json解析失败1"), false);
        return;
    }
    if (!jsonDoc.isObject()) {
        showTip(tr("json解析失败2"), false);
        return;
    }
    jsonDoc.object();
    _handlers[id](jsonDoc.object());
    return;
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

void RegisterDialog::initHttpHandlers()
{
//注册获取验证码回包的逻辑
    _handlers.insert(ReqId::ID_GET_VARIFY_CODE, [this](const QJsonObject& jsonObj) {
        int error = jsonObj["error"].toInt();
        if (error != ErrorCodes::SUCCESS) {
            showTip(tr("参数错误"), false);
            return;
        }
        auto email = jsonObj["email"].toString();
        showTip("验证码已经发送到邮箱", true);
        qDebug() << "email:" << email;
    });
}

