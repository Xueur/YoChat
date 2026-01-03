#ifndef HTTPMGR_H
#define HTTPMGR_H
#include "singleton.h"
#include <QObject>
#include <QString>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QJsonObject>
#include <QJsonDocument>
//CRTP
class HttpMgr:public QObject, public Singleton<HttpMgr>, public std::enable_shared_from_this<HttpMgr>
{
    Q_OBJECT
public:
    ~HttpMgr();
     void PostHttpReq(QUrl url, QJsonObject json, ReqId req_id, Modules mod);
private:
    friend class Singleton<HttpMgr>;
    HttpMgr();
    QNetworkAccessManager _manager;
private slots:
    void slot_http_finish(ReqId id, QString res, ErrorCodes err, Modules mod);
signals:
    void sig_http_finish(ReqId id, QString res, ErrorCodes err, Modules mod);
    void sig_reg_mod_finish(ReqId id, QString res, ErrorCodes err);
};

#endif // HTTPMGR_H
