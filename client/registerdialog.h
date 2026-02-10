#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>
#include "global.h"

namespace Ui {
class RegisterDialog;
}

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget *parent = nullptr);
    ~RegisterDialog();

private slots:
    void on_verify_button_clicked();
    void slot_reg_mod_finish(ReqId id, QString res, ErrorCodes err);
    void on_yes_button_clicked();
    void on_no_button_clicked();

    void on_return_button_clicked();

private:
    Ui::RegisterDialog *ui;
    void showTip(QString str, bool state);
    void initHttpHandlers();
    bool checkUserValid();
    bool checkEmailValid();
    bool checkPwdValid();
    bool checkVerifyValid();
    void AddTipErr(TipErr te, QString tips);
    void DelTipErr(TipErr te);
    void ChangeTipPage();
    QMap<ReqId, std::function<void(const QJsonObject&)>> _handlers;
    QMap<TipErr, QString> _tip_errs;

    QTimer* _countdown_timer;
    int _countdown;
signals:
    void sigSwitchLogin();
};

#endif // REGISTERDIALOG_H
