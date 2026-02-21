#ifndef TCPMGR_H
#define TCPMGR_H
#include <QTcpSocket>
#include "singleton.h"
#include "global.h"
class TcpMgr:public QObject, public Singleton<TcpMgr>,
               public std::enable_shared_from_this<TcpMgr>
{
    Q_OBJECT
    friend class Singleton<TcpMgr>;
public:
    ~TcpMgr();
private:
    TcpMgr();
    void initHandlers();
    void handleMsg();
    QTcpSocket _socket;
    QString _host;
    uint16_t _port;
    QByteArray _buffer;
    bool _b_recv_pending;
    quint16 _message_id;
    quint16 _message_len;
    QMap<ReqId, std::function<void(ReqId id, int len, QByteArray data)>> _handlers;
public slots:
    void slot_tcp_connect(ServerInfo);
    void slot_send_data(ReqId reqId, QString data);
signals:
    void sig_con_success(bool bsuccess);
    void sig_send_data(ReqId reqId, QString data);
    void sig_switch_chatDlg();
    void sig_login_failed(int);
};

#endif // TCPMGR_H
