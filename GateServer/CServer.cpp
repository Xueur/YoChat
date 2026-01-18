#include "CServer.h"
#include "HttpConnection.h"
#include "AsioIOServicePool.h"
CServer::CServer(net::io_context& ioc, unsigned short& port)
    :_ioc(ioc),
    _acceptor(ioc, tcp::endpoint(tcp::v4(), port))
{

}

void CServer::start() {
    auto self = shared_from_this();
    auto& io_context = AsioIOServicePool::getInstance()->GetIOService();
    std::shared_ptr<HttpConnection> new_con = std::make_shared<HttpConnection>(io_context);
    _acceptor.async_accept(new_con->getSocket(), [self, new_con](beast::error_code ec) {
        try {
            if (!ec) new_con->start();
            self->start();
        }
        catch (std::exception& exp) {
            std::cout << "exception is " << exp.what() << std::endl;
            self->start();
        }
    });
}