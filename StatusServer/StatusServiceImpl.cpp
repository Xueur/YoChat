#include "StatusServiceImpl.h"
#include "ConfigMgr.h"
#include "RedisMgr.h"
#include "common.h"

std::string generate_unique_string() {
	// 创建UUID对象
	boost::uuids::uuid uuid = boost::uuids::random_generator()();

	// 将UUID转换为字符串
	std::string unique_string = to_string(uuid);

	return unique_string;
}

Status StatusServiceImpl::GetChatServer(ServerContext* context, const GetChatServerReq* request, GetChatServerRsp* reply)
{
	std::string prefix("llfc status server has received :  ");
	const auto& server = getChatServer();
	reply->set_host(server.host);
	reply->set_port(server.port);
	reply->set_error(ErrorCodes::Success);
	reply->set_token(generate_unique_string());
	insertToken(request->uid(), reply->token());
	return Status::OK;
}

StatusServiceImpl::StatusServiceImpl()
{
	auto& cfg = ConfigMgr::getInstance();
	ChatServer server;
	server.port = cfg["ChatServer1"]["Port"];
	server.host = cfg["ChatServer1"]["Host"];
	server.con_count = 0;
	server.name = cfg["ChatServer1"]["Name"];
	_servers[server.name] = server;

	server.port = cfg["ChatServer2"]["Port"];
	server.host = cfg["ChatServer2"]["Host"];
	server.con_count = 0;
	server.name = cfg["ChatServer2"]["Name"];
	_servers[server.name] = server;
}

ChatServer StatusServiceImpl::getChatServer() {
	std::lock_guard<std::mutex> guard(_server_mtx);
	auto minServer = _servers.begin()->second;
    auto count_str = RedisMgr::getInstance()->HGet(LOGIN_COUNT, minServer.name);
    if (!count_str.empty()) {
		minServer.con_count = std::stoi(count_str);
    }
	// 使用范围基于for循环
	for (auto& server : _servers) {
        if (server.second.name == minServer.name) {
			continue; // 跳过当前最小连接数的服务器
		}
		auto count_str = RedisMgr::getInstance()->HGet(LOGIN_COUNT, server.second.name);
		if (!count_str.empty()) {
			server.second.con_count = std::stoi(count_str);
		}
		if (server.second.con_count < minServer.con_count) {
			minServer = server.second;
		}
	}
	return minServer;
}

Status StatusServiceImpl::Login(ServerContext* context, const LoginReq* request, LoginRsp* reply)
{
	auto uid = request->uid();
	auto token = request->token();
    std::string token_key = USERTOKENPREFIX + std::to_string(uid);
	std::string token_value = "";
    bool success = RedisMgr::getInstance()->Get(token_key, token_value);
	if (!success) {
		reply->set_error(ErrorCodes::UidInvalid);
        return Status::OK;
	}
	if (token_value != token) {
		reply->set_error(ErrorCodes::TokenInvalid);
		return Status::OK;
    }
	reply->set_error(ErrorCodes::Success);
	reply->set_uid(uid);
	reply->set_token(token);
	return Status::OK;
}

void StatusServiceImpl::insertToken(int uid, std::string token)
{
	std::string token_key = USERTOKENPREFIX + std::to_string(uid);
    RedisMgr::getInstance()->Set(token_key, token);
}