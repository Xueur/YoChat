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
    ui->err_tip->clear();
    connect(ui->user_edit,&QLineEdit::editingFinished,this,[this](){
        checkUserValid();
    });
    connect(ui->email_edit, &QLineEdit::editingFinished, this, [this](){
        checkEmailValid();
    });
    connect(ui->pwd_edit, &QLineEdit::editingFinished, this, [this](){
        checkPwdValid();
    });
    // connect(ui->confirm_edit, &QLineEdit::editingFinished, this, [this](){
    //     checkConfirmValid();
    // });
    connect(ui->verify_edit, &QLineEdit::editingFinished, this, [this](){
        checkVerifyValid();
    });

    //设置浮动显示眼睛形状
    ui->pwd_visible->setCursor(Qt::PointingHandCursor);
    ui->confirm_visible->setCursor(Qt::PointingHandCursor);
    ui->pwd_visible->SetState("unvisible","unvisible_hover","","visible",
                               "visible_hover","");
    ui->confirm_visible->SetState("unvisible","unvisible_hover","","visible",
                                  "visible_hover","");
    // 连接点击事件
    connect(ui->pwd_visible, &ClickedLabel::clicked, this, [this]() {
        auto state = ui->pwd_visible->GetCurState();
        if(state == ClickLbState::Normal){
            ui->pwd_edit->setEchoMode(QLineEdit::Password);
        }else{
            ui->pwd_edit->setEchoMode(QLineEdit::Normal);
        }
        qDebug() << "Label was clicked!";
    });
    connect(ui->confirm_visible, &ClickedLabel::clicked, this, [this]() {
        auto state = ui->confirm_visible->GetCurState();
        if(state == ClickLbState::Normal){
            ui->confirm_edit->setEchoMode(QLineEdit::Password);
        }else{
            ui->confirm_edit->setEchoMode(QLineEdit::Normal);
        }
        qDebug() << "Label was clicked!";
    });
    // 创建定时器
    _countdown_timer = new QTimer(this);
    // 连接信号和槽
    connect(_countdown_timer, &QTimer::timeout, [this](){
        if(_countdown==0){
            _countdown_timer->stop();
            emit sigSwitchLogin();
            return;
        }
        _countdown--;
        // auto str = QString("注册成功，%1 s后返回登录").arg(_countdown);
        // ui->tip_lb->setText(str);
    });
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
         HttpMgr::getInstance()->PostHttpReq(QUrl(gate_url_prefix+"/get_verifycode"),
                                             json_obj, ReqId::ID_GET_VERIFY_CODE,Modules::REGISTERMOD);
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
    _handlers.insert(ReqId::ID_GET_VERIFY_CODE, [this](const QJsonObject& jsonObj) {
        int error = jsonObj["error"].toInt();
        if (error != ErrorCodes::SUCCESS) {
            showTip(tr("参数错误"), false);
            return;
        }
        auto email = jsonObj["email"].toString();
        showTip("验证码已经发送到邮箱", true);
        qDebug() << "email:" << email;
    });
    //注册注册用户回包逻辑
    _handlers.insert(ReqId::ID_REG_USER, [this](QJsonObject jsonObj){
        int error = jsonObj["error"].toInt();
        if(error != ErrorCodes::SUCCESS){
            showTip(tr("参数错误"),false);
            return;
        }
        auto email = jsonObj["email"].toString();
        showTip(tr("用户注册成功"), true);
        qDebug()<< "email is " << email ;
    });
}

bool RegisterDialog::checkUserValid()
{
    if(ui->user_edit->text() == ""){
        AddTipErr(TipErr::TIP_USER_ERR, tr("用户名不能为空"));
        return false;
    }
    DelTipErr(TipErr::TIP_USER_ERR);
    return true;
}

bool RegisterDialog::checkEmailValid()
{

    //验证邮箱的地址正则表达式
    auto email = ui->email_edit->text();
    // 邮箱地址的正则表达式
    QRegularExpression regex(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)");
    bool match = regex.match(email).hasMatch(); // 执行正则表达式匹配
    if(!match){
        //提示邮箱不正确
        AddTipErr(TipErr::TIP_EMAIL_ERR, tr("邮箱地址不正确"));
        return false;
    }
    DelTipErr(TipErr::TIP_EMAIL_ERR);
    return true;
}

bool RegisterDialog::checkPwdValid()
{
    auto pass = ui->pwd_edit->text();
    if(pass.length() < 6 || pass.length()>15){
        //提示长度不准确
        AddTipErr(TipErr::TIP_PWD_ERR, tr("密码长度应为6~15"));
        return false;
    }
    // 创建一个正则表达式对象，按照上述密码要求
    // 这个正则表达式解释：
    // ^[a-zA-Z0-9!@#$%^&*]{6,15}$ 密码长度至少6，可以是字母、数字和特定的特殊字符
    QRegularExpression regExp("^[a-zA-Z0-9!@#$%^&*]{6,15}$");
    bool match = regExp.match(pass).hasMatch();
    if(!match){
        //提示字符非法
        AddTipErr(TipErr::TIP_PWD_ERR, tr("不能包含非法字符"));
        return false;;
    }
    DelTipErr(TipErr::TIP_PWD_ERR);
    return true;
}

bool RegisterDialog::checkVerifyValid()
{

    auto pass = ui->verify_edit->text();
    if(pass.isEmpty()){
        AddTipErr(TipErr::TIP_VARIFY_ERR, tr("验证码不能为空"));
        return false;
    }
    DelTipErr(TipErr::TIP_VARIFY_ERR);
    return true;
}

void RegisterDialog::AddTipErr(TipErr te, QString tips)
{
    _tip_errs[te] = tips;
    showTip(tips, false);
}
void RegisterDialog::DelTipErr(TipErr te)
{
    _tip_errs.remove(te);
    if(_tip_errs.empty()){
        ui->err_tip->clear();
        return;
    }
    showTip(_tip_errs.first(), false);
}

void RegisterDialog::ChangeTipPage()
{
    _countdown_timer->stop();
    ui->stackedWidget->setCurrentWidget((ui->page_2));
    _countdown_timer->start(5000);
}

void RegisterDialog::on_yes_button_clicked()
{
    if (!checkUserValid() || !checkEmailValid() || !checkPwdValid() || !checkVerifyValid()) return;
    if(ui->confirm_edit->text() != ui->pwd_edit->text()){
        showTip(tr("密码和确认密码不匹配"), false);
        return;
    }
    QJsonObject json_obj;
    json_obj["user"] = ui->user_edit->text();
    json_obj["email"] = ui->email_edit->text();
    json_obj["pwd"] = xorString(ui->pwd_edit->text());
    json_obj["confirm"] = xorString(ui->confirm_edit->text());
    json_obj["verifycode"] = ui->verify_edit->text();
    HttpMgr::getInstance()->PostHttpReq(QUrl(gate_url_prefix+"/user_register"),
                                        json_obj, ReqId::ID_REG_USER,Modules::REGISTERMOD);
    ChangeTipPage();
}


void RegisterDialog::on_no_button_clicked()
{
    _countdown_timer->stop();
    emit sigSwitchLogin();
}


void RegisterDialog::on_return_button_clicked()
{
    _countdown_timer->stop();
    emit sigSwitchLogin();
}

