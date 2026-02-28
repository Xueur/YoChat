#include "LogicSystem.h"
#include "StatusGrpcClient.h"
#include "MysqlMgr.h"
#include "RedisMgr.h"
#include "UserMgr.h"
#include "common.h"

using namespace std;

LogicSystem::LogicSystem() :_b_stop(false) {
	RegisterCallBacks();
	_worker_thread = std::thread(&LogicSystem::DealMsg, this);
}

LogicSystem::~LogicSystem() {
	_b_stop = true;
	_consume.notify_one();
	_worker_thread.join();
}

void LogicSystem::PostMsgToQue(shared_ptr < LogicNode> msg) {
	std::unique_lock<std::mutex> unique_lk(_mutex);
	_msg_que.push(msg);
	//由0变为1则发送通知信号
	if (_msg_que.size() == 1) {
		unique_lk.unlock();
		_consume.notify_one();
	}
}

void LogicSystem::DealMsg() {
	while(true) {
		std::unique_lock<std::mutex> unique_lk(_mutex);
		//判断队列为空则用条件变量阻塞等待，并释放锁
		_consume.wait(unique_lk, [this] { return !_msg_que.empty() || _b_stop; });
		//判断是否为关闭状态，把所有逻辑执行完后则退出循环
		if (_b_stop) {
			while (!_msg_que.empty()) {
				auto msg_node = _msg_que.front();
				cout << "recv_msg id  is " << msg_node->_recvnode->_msg_id << endl;
				auto call_back_iter = _fun_callbacks.find(msg_node->_recvnode->_msg_id);
				if (call_back_iter == _fun_callbacks.end()) {
					_msg_que.pop();
					continue;
				}
				call_back_iter->second(msg_node->_session, msg_node->_recvnode->_msg_id,
					std::string(msg_node->_recvnode->_data, msg_node->_recvnode->_cur_len));
				_msg_que.pop();
			}
			break;
		}

		//没有退出说明队列中有数据
		auto msg_node = _msg_que.front();
		cout << "recv_msg id  is " << msg_node->_recvnode->_msg_id << endl;
		auto call_back_iter = _fun_callbacks.find(msg_node->_recvnode->_msg_id);
		if (call_back_iter == _fun_callbacks.end()) {
			_msg_que.pop();
			std::cout << "msg id [" << msg_node->_recvnode->_msg_id << "] handler not found" << std::endl;
			continue;
		}
		call_back_iter->second(msg_node->_session, msg_node->_recvnode->_msg_id,
			std::string(msg_node->_recvnode->_data, msg_node->_recvnode->_cur_len));
		_msg_que.pop();
	}
}

void LogicSystem::RegisterCallBacks() {
	_fun_callbacks[MSG_CHAT_LOGIN] = std::bind(&LogicSystem::LoginHandler, this,
		placeholders::_1, placeholders::_2, placeholders::_3);
}

void LogicSystem::LoginHandler(shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {
	Json::Reader reader;
    Json::Value root;//root对象用来存储解析后的json数据，存的是客户端发送过来的数据
	reader.parse(msg_data, root);
	auto uid = root["uid"].asInt();
	auto token = root["token"].asString();
	std::cout << "user login uid is  " << uid << " user token  is "
		<< token << endl;

	Json::Value  rtvalue;
	Defer defer([this, &rtvalue, session]() {
		std::string return_str = rtvalue.toStyledString();
		session->Send(return_str, MSG_CHAT_LOGIN_RSP);
		});
	//从redis获取token匹配是否准确
    std::string token_key = USERTOKENPREFIX + std::to_string(uid);
	std::string token_value = "";
    bool success = RedisMgr::getInstance()->Get(token_key, token_value);
	if (!success) {
        rtvalue["error"] = ErrorCodes::TokenInvalid;
		return;
	}
	if (token_value != token) {
		rtvalue["error"] = ErrorCodes::TokenInvalid;
		return;
	}
    rtvalue["error"] = ErrorCodes::Success;

	std::string base_key = USER_BASE_INFO + std::to_string(uid);
	auto user_info = std::make_shared<UserInfo>();
	bool b_base = GetBaseInfo(base_key, uid, user_info);
	if (!b_base) {
		rtvalue["error"] = ErrorCodes::UidInvalid;
		return;
	}
	rtvalue["uid"] = uid;
	rtvalue["pwd"] = user_info->pwd;
	rtvalue["name"] = user_info->name;
	rtvalue["email"] = user_info->email;
	rtvalue["nick"] = user_info->nick;
	rtvalue["desc"] = user_info->desc;
	rtvalue["sex"] = user_info->sex;
	rtvalue["icon"] = user_info->icon;

	//获取申请列表

	//获取好友列表
    auto server_name = ConfigMgr::getInstance().GetValue("SelfServer", "Name");
	//增加登录数量
    auto rd_res = RedisMgr::getInstance()->HGet(LOGIN_COUNT, server_name);
    int count = 0;
	if (!rd_res.empty()) {
		count = std::stoi(rd_res);
    }
    count++;
    RedisMgr::getInstance()->HSet(LOGIN_COUNT, server_name, std::to_string(count));
    //session绑定uid
	session->SetUserId(uid);
	//为用户设置登录ip_server的名字
	auto ip_key = USERIPPREFIX + std::to_string(uid);
    RedisMgr::getInstance()->Set(ip_key, server_name);
	//uid和session绑定管理
    UserMgr::getInstance()->SetUserSession(uid, session);
}

bool LogicSystem::GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo)
{
    //先从redis获取，如果没有则从mysql获取，并更新到redis中
    std::string user_info_str = "";
    bool success = RedisMgr::getInstance()->Get(base_key, user_info_str);
	if (success) {
		Json::Reader reader;
		Json::Value root;
		reader.parse(user_info_str, root);
		userinfo->uid = root["uid"].asInt();
		userinfo->pwd = root["pwd"].asString();
		userinfo->name = root["name"].asString();
		userinfo->email = root["email"].asString();
		userinfo->nick = root["nick"].asString();
		userinfo->desc = root["desc"].asString();
		userinfo->icon = root["icon"].asString();
		userinfo->sex = root["sex"].asInt();
		std::cout << "user uid is " << userinfo->uid << " user email is " << userinfo->email << endl;
	} else {
		std::shared_ptr<UserInfo> user_info_mysql = MysqlMgr::getInstance()->getUser(uid);
		if (user_info_mysql == nullptr) {
			return false;
		}
		userinfo = user_info_mysql;
		Json::Value root;
		root["uid"] = userinfo->uid;
		root["pwd"] = userinfo->pwd;
		root["name"] = userinfo->name;
		root["email"] = userinfo->email;
		root["nick"] = userinfo->nick;
		root["desc"] = userinfo->desc;
		root["icon"] = userinfo->icon;
		root["sex"] = userinfo->sex;
        RedisMgr::getInstance()->Set(base_key, root.toStyledString());
    }
}
