#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include "global.h"
#include "tcpmgr.h"

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

private slots:
    void slot_forget_pwd();

    void on_login_btn_clicked();

    void slot_login_mod_finish(ReqId id, QString res, ErrorCodes err);

    void slot_tcp_con_finish(bool bsuccess);

    void slot_chat_login_finish();

private:
    Ui::LoginDialog *ui;
    void initHttpHandlers();
    bool checkEmailValid();
    bool checkPwdValid();
    bool enableBtn(bool enabled);
    void showTip(QString str, bool b_ok);
    void AddTipErr(TipErr te, QString tips);
    void DelTipErr(TipErr te);
    QMap<TipErr, QString> _tip_errs;
    QMap<ReqId, std::function<void(const QJsonObject&)>> _handlers;
    int _uid;
    QString _token;
signals:
    void switchRegister();
    void switchReset();
    void sig_connect_tcp(ServerInfo);
    void sig_send_data(ReqId reqId, QString data);
};

#endif // LOGINDIALOG_H
