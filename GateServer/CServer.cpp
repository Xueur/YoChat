#include "CServer.h"
#include "HttpConnection.h"
CServer::CServer(net::io_context& ioc, unsigned short& port)
    :_ioc(ioc),
    _acceptor(net::make_strand(ioc), tcp::endpoint(tcp::v4(), port)),
    _socket(net::make_strand(ioc))
{

}

void CServer::start() {
    auto self = shared_from_this();
    _acceptor.async_accept(_socket, [self](beast::error_code ec) {
        try {
            if (ec) {
                self->start();
                return;
            }
            std::make_shared<HttpConnection>(std::move(self->_socket))->start();
            self->start();
        }
        catch (std::exception& exp) {
            std::cout << "exception is " << exp.what() << std::endl;
            self->start();
        }
    });
}