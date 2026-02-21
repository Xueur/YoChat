#include "tcpmgr.h"
#include <QJsonDocument>
#include <QJsonObject>
TcpMgr::~TcpMgr()
{

}

TcpMgr::TcpMgr():_host(""),_port(0),_b_recv_pending(false),_message_id(0),_message_len(0)
{
    QObject::connect(&_socket, &QTcpSocket::connected, [&]() {
        qDebug() << "Connected to server!";
        // 连接建立后发送消息
        emit sig_con_success(true);
    });
    QObject::connect(&_socket, &QTcpSocket::readyRead, [&]() {
        // 当有数据可读时，读取所有数据
        // 读取所有数据并追加到缓冲区
        _buffer.append(_socket.readAll());
        QDataStream stream(&_buffer, QIODevice::ReadOnly);
        stream.setVersion(QDataStream::Qt_5_0);
        forever {
            //先解析头部
            if(!_b_recv_pending){
                // 检查缓冲区中的数据是否足够解析出一个消息头（消息ID + 消息长度）
                if (_buffer.size() < static_cast<int>(sizeof(quint16) * 2)) {
                    return; // 数据不够，等待更多数据
                }
                // 预读取消息ID和消息长度，但不从缓冲区中移除
                stream >> _message_id >> _message_len;
                //将buffer 中的前四个字节移除
                _buffer = _buffer.mid(sizeof(quint16) * 2);
                // 输出读取的数据
                qDebug() << "Message ID:" << _message_id << ", Length:" << _message_len;
            }
            //buffer剩余长读是否满足消息体长度，不满足则退出继续等待接受
            if(_buffer.size() < _message_len){
                _b_recv_pending = true;
                return;
            }
            _b_recv_pending = false;
            // 读取消息体
            QByteArray messageBody = _buffer.mid(0, _message_len);
            qDebug() << "receive body msg is " << messageBody ;
            _buffer = _buffer.mid(_message_len);
        }
    });

    // 处理错误
    QObject::connect(&_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred), [&](QAbstractSocket::SocketError socketError) {
        Q_UNUSED(socketError)
        qDebug() << "Error:" << _socket.errorString();
    });

    // 处理连接断开
    QObject::connect(&_socket, &QTcpSocket::disconnected, [&]() {
        qDebug() << "Disconnected from server.";
    });
    QObject::connect(this, &TcpMgr::sig_send_data, this, &TcpMgr::slot_send_data);
}

void TcpMgr::initHandlers()
{
    //auto self = shared_from_this();
    _handlers.insert(ID_CHAT_LOGIN_RSP, [this](ReqId id, int len, QByteArray data){
        Q_UNUSED(len);
        qDebug()<< "handle id is "<< id ;
        // 将QByteArray转换为QJsonDocument
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

        // 检查转换是否成功
        if(jsonDoc.isNull()){
            qDebug() << "Failed to create QJsonDocument.";
            return;
        }

        QJsonObject jsonObj = jsonDoc.object();
        qDebug()<< "data jsonobj is " << jsonObj ;

        if(!jsonObj.contains("error")){
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "Login Failed, err is Json Parse Err" << err ;
            emit sig_login_failed(err);
            return;
        }

        int err = jsonObj["error"].toInt();
        if(err != ErrorCodes::SUCCESS){
            qDebug() << "Login Failed, err is " << err ;
            emit sig_login_failed(err);
            return;
        }
    });
}

void TcpMgr::handleMsg()
{

}

void TcpMgr::slot_tcp_connect(ServerInfo si)
{
    qDebug()<< "receive tcp connect signal";
    // 尝试连接到服务器
    qDebug() << "Connecting to server...";
    _host = si.Host;
    _port = static_cast<uint16_t>(si.Port.toUInt());
    _socket.connectToHost(si.Host, _port);
}

void TcpMgr::slot_send_data(ReqId reqId, QString data)
{
    uint16_t id = reqId;
    QByteArray dataBytes = data.toUtf8();
    // 计算长度（使用网络字节序转换）
    quint16 len = static_cast<quint16>(data.size());
    // 创建一个QByteArray用于存储要发送的所有数据
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    // 设置数据流使用网络字节序
    out.setByteOrder(QDataStream::BigEndian);
    out << id << len;
    block.append(dataBytes);
    _socket.write(block);
}
