#include "CServer.h"
#include <iostream>
#include "AsioIOServicePool.h"
CServer::CServer(boost::asio::io_context& io_context, short port) :_io_context(io_context), _port(port),
_acceptor(io_context, tcp::endpoint(tcp::v4(), port))
{
	cout << "Server start success, listen on port : " << _port << endl;
	StartAccept();
}

CServer::~CServer() {
	cout << "Server destruct listen on port : " << _port << endl;
}

void CServer::HandleAccept(shared_ptr<CSession> new_session, const boost::system::error_code& error) {
	if (!error) {
		new_session->Start();
		lock_guard<mutex> lock(_mtx);
		_sessions.insert(make_pair(new_session->getUuid(), new_session));
	}
	else {
		cout << "session accept failed, error is " << error.what() << endl;
	}

	StartAccept();
}

void CServer::StartAccept() {
	auto& io_context = AsioIOServicePool::getInstance()->GetIOService();
	shared_ptr<CSession> new_session = make_shared<CSession>(io_context, this);
	_acceptor.async_accept(new_session->getSocket(), std::bind(&CServer::HandleAccept, this, new_session, placeholders::_1));
}

void CServer::ClearSession(std::string uuid) {
	lock_guard<mutex> lock(_mtx);
	_sessions.erase(uuid);
}