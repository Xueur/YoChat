#include "logindialog.h"
#include "ui_logindialog.h"
#include "httpmgr.h"
#include <QObject>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    connect(ui->register_btn, &QPushButton::clicked, this, &LoginDialog::switchRegister);
    ui->forget_label->SetState("normal","hover","","selected","selected_hover","");
    ui->forget_label->setCursor(Qt::PointingHandCursor);
    connect(ui->forget_label, &ClickedLabel::clicked, this, &LoginDialog::slot_forget_pwd);
    initHttpHandlers();
    connect(HttpMgr::getInstance().get(), &HttpMgr::sig_login_mod_finish, this, &LoginDialog::slot_login_mod_finish);
    connect(this, &LoginDialog::sig_connect_tcp, TcpMgr::getInstance().get(), &TcpMgr::slot_tcp_connect);
    connect(TcpMgr::getInstance().get(), &TcpMgr::sig_con_success, this, &LoginDialog::slot_tcp_con_finish);
    connect(this, &LoginDialog::sig_send_data, TcpMgr::getInstance().get(), &TcpMgr::slot_send_data);
    connect(TcpMgr::getInstance().get(), &TcpMgr::sig_swich_chatdlg, this, &LoginDialog::slot_chat_login_finish);
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::slot_forget_pwd()
{
    qDebug()<<"slot forget pwd";
    emit switchReset();
}


void LoginDialog::on_login_btn_clicked()
{
    qDebug()<<"login btn clicked";
    if(checkEmailValid() == false){
        return;
    }
    if(checkPwdValid() == false){
        return ;
    }
    enableBtn(false);
    auto email = ui->email_edit->text();
    auto pwd = ui->pwd_edit->text();
    //发送http请求登录
    QJsonObject json_obj;
    json_obj["email"] = email;
    json_obj["passwd"] = xorString(pwd);
    HttpMgr::getInstance()->PostHttpReq(QUrl(gate_url_prefix+"/user_login"),
                                        json_obj, ReqId::ID_LOGIN_USER,Modules::LOGINMOD);
}

void LoginDialog::initHttpHandlers()
{
    //注册获取登录回包逻辑
    _handlers.insert(ReqId::ID_LOGIN_USER, [this](QJsonObject jsonObj){
        int error = jsonObj["error"].toInt();
        if(error != ErrorCodes::SUCCESS){
            showTip(tr("参数错误"),false);
            enableBtn(true);
            return;
        }
        auto user = jsonObj["email"].toString();
        showTip(tr("登录成功"), true);
        qDebug()<< "user is " << user ;


        //发送信号通知TcpMgr发送长链接
        ServerInfo si;
        si.Uid = jsonObj["uid"].toInt();
        si.Host = jsonObj["host"].toString();
        si.Port = jsonObj["port"].toString();
        si.Token = jsonObj["token"].toString();
        _uid = si.Uid;
        _token = si.Token;
        qDebug()<< "user is " << user << " uid is " << si.Uid <<" host is "
                 << si.Host << " Port is " << si.Port << " Token is " << si.Token;
        emit sig_connect_tcp(si);
    });
}

bool LoginDialog::checkEmailValid()
{
    auto email = ui->email_edit->text();
    if(email.isEmpty()){
        qDebug() << "Email empty " ;
        return false;
    }
    return true;
}

bool LoginDialog::enableBtn(bool enabled)
{
    ui->login_btn->setEnabled(enabled);
    ui->register_btn->setEnabled(enabled);
    return true;
}

bool LoginDialog::checkPwdValid()
{
    auto pwd = ui->pwd_edit->text();
    if(pwd.length() < 6 || pwd.length() > 15){
        qDebug() << "Pwd length invalid";
        return false;
    }
    QRegularExpression regExp("^[a-zA-Z0-9!@#$%^&*.]{6,15}$");
    if(!regExp.match(pwd).hasMatch()){
        return false;
    }
    return true;
}

void LoginDialog::AddTipErr(TipErr te, QString tips)
{
    _tip_errs[te] = tips;
    showTip(tips, false);
}
void LoginDialog::DelTipErr(TipErr te)
{
    _tip_errs.remove(te);
    if(_tip_errs.empty()){
        ui->err_tip->clear();
        return;
    }
    showTip(_tip_errs.first(), false);
}

void LoginDialog::showTip(QString str, bool b_ok)
{
    if(b_ok){
        ui->err_tip->setProperty("state","normal");
    }else{
        ui->err_tip->setProperty("state","err");
    }

    ui->err_tip->setText(str);

    repolish(ui->err_tip);
}

void LoginDialog::slot_login_mod_finish(ReqId id, QString res, ErrorCodes err)
{
    if(err != ErrorCodes::SUCCESS){
        showTip(tr("网络请求错误"),false);
        return;
    }
    QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());
    //json解析错误
    if(jsonDoc.isNull()){
        showTip(tr("json解析错误"),false);
        return;
    }
    //json解析错误
    if(!jsonDoc.isObject()){
        showTip(tr("json解析错误"),false);
        return;
    }
    //调用对应的逻辑,根据id回调。
    _handlers[id](jsonDoc.object());
    return;
}

void LoginDialog::slot_tcp_con_finish(bool bsuccess)
{
    if(bsuccess){
        showTip(tr("聊天服务连接成功，正在登录..."),true);
        QJsonObject jsonObj;
        jsonObj["uid"] = _uid;
        jsonObj["token"] = _token;
        QJsonDocument doc(jsonObj);
        QString jsonString = doc.toJson(QJsonDocument::Indented);
        //发送tcp请求给chatserver
        emit sig_send_data(ReqId::ID_CHAT_LOGIN, jsonString);
    }else{
        showTip(tr("网络异常"),false);
        enableBtn(true);
    }
}

void LoginDialog::slot_chat_login_finish()
{
    emit switchChat();
}


