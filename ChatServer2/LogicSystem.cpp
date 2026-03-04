#include "LogicSystem.h"
#include "StatusGrpcClient.h"
#include "MysqlMgr.h"
#include "RedisMgr.h"
#include "UserMgr.h"
#include "ChatGrpcClient.h"
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
	_fun_callbacks[ID_SEARCH_USER_REQ] = std::bind(&LogicSystem::SearchInfo, this,
        placeholders::_1, placeholders::_2, placeholders::_3);
    _fun_callbacks[ID_ADD_FRIEND_REQ] = std::bind(&LogicSystem::AddFriendApply, this,
        placeholders::_1, placeholders::_2, placeholders::_3);
	_fun_callbacks[ID_AUTH_FRIEND_REQ] = std::bind(&LogicSystem::AuthFriendApply, this,
		placeholders::_1, placeholders::_2, placeholders::_3);
	_fun_callbacks[ID_TEXT_CHAT_MSG_REQ] = std::bind(&LogicSystem::DealChatTextMsg, this,
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
	std::vector<std::shared_ptr<ApplyInfo>> apply_list;
	auto b_apply = GetFriendApplyInfo(uid, apply_list);
	if (b_apply) {
		for (auto& apply : apply_list) {
			Json::Value obj;
			obj["name"] = apply->_name;
			obj["uid"] = apply->_uid;
			obj["icon"] = apply->_icon;
			obj["nick"] = apply->_nick;
			obj["sex"] = apply->_sex;
			obj["desc"] = apply->_desc;
			obj["status"] = apply->_status;
			rtvalue["apply_list"].append(obj);
		}
	}
	//获取好友列表
	std::vector<std::shared_ptr<UserInfo>> friend_list;
	bool b_friend_list = GetFriendList(uid, friend_list);
	for (auto& friend_ele : friend_list) {
		Json::Value obj;
		obj["name"] = friend_ele->name;
		obj["uid"] = friend_ele->uid;
		obj["icon"] = friend_ele->icon;
		obj["nick"] = friend_ele->nick;
		obj["sex"] = friend_ele->sex;
		obj["desc"] = friend_ele->desc;
		rtvalue["friend_list"].append(obj);
	}

    
	//增加登录数量
	auto server_name = ConfigMgr::getInstance().GetValue("SelfServer", "Name");
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

void LogicSystem::SearchInfo(shared_ptr<CSession> session, const short& msg_id, const string& msg_data)
{
	Json::Reader reader;
	Json::Value root;
	reader.parse(msg_data, root);
	auto uid_str = root["uid"].asString();
	std::cout << "user SearchInfo uid is  " << uid_str << endl;
	Json::Value rtvalue;
	Defer defer([this, &rtvalue, session]() {
		std::string return_str = rtvalue.toStyledString();
		session->Send(return_str, ID_SEARCH_USER_RSP);
		});
	bool b_digit = isPureDigit(uid_str);
	if (b_digit) {
		GetUserByUid(uid_str, rtvalue);
	}
	else {
		GetUserByName(uid_str, rtvalue);
	}
}

void LogicSystem::AddFriendApply(shared_ptr<CSession> session, const short& msg_id, const string& msg_data)
{
	Json::Reader reader;
	Json::Value root;
	reader.parse(msg_data, root);
	auto uid = root["uid"].asInt();
	auto desc = root["desc"].asString();
	auto rmkname = root["rmkname"].asString();
	auto touid = root["touid"].asInt();
	std::cout << "user login uid is  " << uid << " desc  is "
		<< desc << " rmkname is " << rmkname << " touid is " << touid << endl;
	Json::Value  rtvalue;
	rtvalue["error"] = ErrorCodes::Success;
	Defer defer([this, &rtvalue, session]() {
		std::string return_str = rtvalue.toStyledString();
		session->Send(return_str, ID_ADD_FRIEND_RSP);
		});
	//先更新数据库
	MysqlMgr::getInstance()->AddFriendApply(uid, touid);
	//查询redis 查找touid对应的server ip
	auto to_ip_key = USERIPPREFIX + std::to_string(touid);
	std::string to_ip_value = "";
	bool b_ip = RedisMgr::getInstance()->Get(to_ip_key, to_ip_value);
	if (!b_ip) {
		return;
	}
	auto& cfg = ConfigMgr::getInstance();
	auto self_name = cfg["SelfServer"]["Name"];
    auto applyname = MysqlMgr::getInstance()->getUser(uid)->name;
	//直接通知对方有申请消息
	if (to_ip_value == self_name) {
		auto session = UserMgr::getInstance()->GetSession(touid);
		if (session) {
			//在内存中则直接发送通知对方
			Json::Value  notify;
			notify["error"] = ErrorCodes::Success;
			notify["applyuid"] = uid;
			notify["applyname"] = applyname;
			notify["desc"] = desc;
			std::string return_str = notify.toStyledString();
			session->Send(return_str, ID_NOTIFY_ADD_FRIEND_REQ);
		}
		return;
	}
	std::string base_key = USER_BASE_INFO + std::to_string(uid);
	auto apply_info = std::make_shared<UserInfo>();
	bool b_info = GetBaseInfo(base_key, uid, apply_info);
	AddFriendReq add_req;
	add_req.set_applyuid(uid);
	add_req.set_touid(touid);
	add_req.set_name(applyname);
	add_req.set_desc(desc);
	if (b_info) {
		add_req.set_icon(apply_info->icon);
		add_req.set_sex(apply_info->sex);
		add_req.set_nick(apply_info->nick);
	}
	//发送通知
	cout << applyname << " " << desc << endl;
	ChatGrpcClient::getInstance()->NotifyAddFriend(to_ip_value, add_req);
}

void LogicSystem::AuthFriendApply(std::shared_ptr<CSession> session, const short& msg_id, const string& msg_data)
{
	Json::Reader reader;
	Json::Value root;
	reader.parse(msg_data, root);
	auto uid = root["fromuid"].asInt();
	auto touid = root["touid"].asInt();
	auto rmkname = root["rmkname"].asString();
	std::cout << "from " << uid << " auth friend to " << touid << std::endl;
	Json::Value  rtvalue;
	rtvalue["error"] = ErrorCodes::Success;
	auto user_info = std::make_shared<UserInfo>();
	std::string base_key = USER_BASE_INFO + std::to_string(touid);
	bool b_info = GetBaseInfo(base_key, touid, user_info);
	if (b_info) {
		rtvalue["name"] = user_info->name;
		rtvalue["nick"] = user_info->nick;
		rtvalue["icon"] = user_info->icon;
		rtvalue["sex"] = user_info->sex;
		rtvalue["uid"] = touid;
	}
	else {
		rtvalue["error"] = ErrorCodes::UidInvalid;
	}
	Defer defer([this, &rtvalue, session]() {
		std::string return_str = rtvalue.toStyledString();
		session->Send(return_str, ID_AUTH_FRIEND_RSP);
		});
	//先更新数据库
	MysqlMgr::getInstance()->AuthFriendApply(uid, touid);
	//更新数据库添加好友
	MysqlMgr::getInstance()->AddFriend(uid, touid, rmkname);
	//查询redis 查找touid对应的server ip
	auto to_str = std::to_string(touid);
	auto to_ip_key = USERIPPREFIX + to_str;
	std::string to_ip_value = "";
	bool b_ip = RedisMgr::getInstance()->Get(to_ip_key, to_ip_value);
	if (!b_ip) {
		return;
	}
	std::cout << "why2" << endl;
	auto& cfg = ConfigMgr::getInstance();
	auto self_name = cfg["SelfServer"]["Name"];
	//直接通知对方有认证通过消息
	if (to_ip_value == self_name) {
		auto session = UserMgr::getInstance()->GetSession(touid);
		if (session) {
			//在内存中则直接发送通知对方
			Json::Value  notify;
			notify["error"] = ErrorCodes::Success;
			notify["fromuid"] = uid;
			notify["touid"] = touid;
			std::string base_key = USER_BASE_INFO + std::to_string(uid);
			auto user_info = std::make_shared<UserInfo>();
			bool b_info = GetBaseInfo(base_key, uid, user_info);
			if (b_info) {
				notify["name"] = user_info->name;
				notify["nick"] = user_info->nick;
				notify["icon"] = user_info->icon;
				notify["sex"] = user_info->sex;
			}
			else {
				notify["error"] = ErrorCodes::UidInvalid;
			}
			std::string return_str = notify.toStyledString();
			session->Send(return_str, ID_NOTIFY_AUTH_FRIEND_REQ);
		}
		return;
	}
	AuthFriendReq auth_req;
	auth_req.set_fromuid(uid);
	auth_req.set_touid(touid);
    cout << "2from " << uid << " auth 2friend to " << touid << std::endl;
	//发送通知
	ChatGrpcClient::getInstance()->NotifyAuthFriend(to_ip_value, auth_req);
}

void LogicSystem::DealChatTextMsg(std::shared_ptr<CSession> session, const short& msg_id, const string& msg_data)
{	
	Json::Reader reader;
	Json::Value root;
	reader.parse(msg_data, root);
	auto uid = root["fromuid"].asInt();
	auto touid = root["touid"].asInt();
	const Json::Value  arrays = root["text_array"];
	Json::Value  rtvalue;
	rtvalue["error"] = ErrorCodes::Success;
	rtvalue["text_array"] = arrays;
	rtvalue["fromuid"] = uid;
	rtvalue["touid"] = touid;
	Defer defer([this, &rtvalue, session]() {
		std::string return_str = rtvalue.toStyledString();
		session->Send(return_str, ID_TEXT_CHAT_MSG_RSP);
		});
	//查询redis 查找touid对应的server ip
	auto to_str = std::to_string(touid);
	auto to_ip_key = USERIPPREFIX + to_str;
	std::string to_ip_value = "";
	bool b_ip = RedisMgr::getInstance()->Get(to_ip_key, to_ip_value);
	if (!b_ip) {
		return;
	}
	auto& cfg = ConfigMgr::getInstance();
	auto self_name = cfg["SelfServer"]["Name"];
	//直接通知对方有认证通过消息
	if (to_ip_value == self_name) {
		auto session = UserMgr::getInstance()->GetSession(touid);
		if (session) {
			//在内存中则直接发送通知对方
			std::string return_str = rtvalue.toStyledString();
			session->Send(return_str, ID_NOTIFY_TEXT_CHAT_MSG_REQ);
		}
		return;
	}
	TextChatMsgReq text_msg_req;
	text_msg_req.set_fromuid(uid);
	text_msg_req.set_touid(touid);
	for (const auto& txt_obj : arrays) {
		auto content = txt_obj["content"].asString();
		auto msgid = txt_obj["msgid"].asString();
		std::cout << "content is " << content << std::endl;
		std::cout << "msgid is " << msgid << std::endl;
		auto* text_msg = text_msg_req.add_textmsgs();
		text_msg->set_msgid(msgid);
		text_msg->set_msgcontent(content);
	}
	//发送通知 todo...
	ChatGrpcClient::getInstance()->NotifyTextChatMsg(to_ip_value, text_msg_req, rtvalue);
}

bool LogicSystem::GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo)
{
    //先从redis获取，如果没有则从mysql获取，并更新到redis中
    std::string user_info_str = "";
    //bool success = RedisMgr::getInstance()->Get(base_key, user_info_str);
	bool success = false;
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

void LogicSystem::GetUserByUid(std::string uid_str, Json::Value& rtvalue)
{
    rtvalue["error"] = ErrorCodes::Success;
    std::string base_key = USER_BASE_INFO + uid_str;
	//优先从redis获取，如果没有则从mysql获取，并更新到redis中
    std::string info_str = "";
    bool b_base = RedisMgr::getInstance()->Get(base_key, info_str);
	if (b_base) {
		Json::Reader reader;
		Json::Value root;
		reader.parse(info_str, root);
		rtvalue["uid"] = root["uid"].asInt();
		rtvalue["pwd"] = root["pwd"].asString();
		rtvalue["name"] = root["name"].asString();
		rtvalue["email"] = root["email"].asString();
		rtvalue["nick"] = root["nick"].asString();
		rtvalue["desc"] = root["desc"].asString();
		rtvalue["sex"] = root["sex"].asInt();
        rtvalue["icon"] = root["icon"].asString();
        std::cout << "user uid is " << rtvalue["uid"].asInt() << " user email is " << rtvalue["email"].asString() << endl;
		return;
	}
    auto uid = std::stoi(uid_str);
    std::shared_ptr<UserInfo> user_info = MysqlMgr::getInstance()->getUser(uid);
	if (user_info == nullptr) {
		rtvalue["error"] = ErrorCodes::UidInvalid;
		return;
	}
    rtvalue["uid"] = user_info->uid;
    rtvalue["pwd"] = user_info->pwd;
    rtvalue["name"] = user_info->name;
    rtvalue["email"] = user_info->email;
    rtvalue["nick"] = user_info->nick;
	rtvalue["desc"] = user_info->desc;
    rtvalue["sex"] = user_info->sex;
    rtvalue["icon"] = user_info->icon;
	//更新到redis中
	Json::Value root;
	root["uid"] = user_info->uid;
	root["pwd"] = user_info->pwd;
	root["name"] = user_info->name;
	root["email"] = user_info->email;
	root["nick"] = user_info->nick;
	root["desc"] = user_info->desc;
	root["icon"] = user_info->icon;
	root["sex"] = user_info->sex;
	RedisMgr::getInstance()->Set(base_key, root.toStyledString());
}

void LogicSystem::GetUserByName(std::string name_str, Json::Value& rtvalue)
{
	rtvalue["error"] = ErrorCodes::Success;
	std::string base_key = USER_BASE_INFO + name_str;
	//优先从redis获取，如果没有则从mysql获取，并更新到redis中
	std::string info_str = "";
	bool b_base = RedisMgr::getInstance()->Get(base_key, info_str);
	if (b_base) {
		Json::Reader reader;
		Json::Value root;
		reader.parse(info_str, root);
		rtvalue["uid"] = root["uid"].asInt();
		rtvalue["pwd"] = root["pwd"].asString();
		rtvalue["name"] = root["name"].asString();
		rtvalue["email"] = root["email"].asString();
		rtvalue["nick"] = root["nick"].asString();
		rtvalue["desc"] = root["desc"].asString();
		rtvalue["sex"] = root["sex"].asInt();
		rtvalue["icon"] = root["icon"].asString();
		std::cout << "user uid is " << rtvalue["uid"].asInt() << " user email is " << rtvalue["email"].asString() << endl;
		return;
	}
	std::shared_ptr<UserInfo> user_info = MysqlMgr::getInstance()->getUser(name_str);
	if (user_info == nullptr) {
		rtvalue["error"] = ErrorCodes::UidInvalid;
		return;
	}
	rtvalue["uid"] = user_info->uid;
	rtvalue["pwd"] = user_info->pwd;
	rtvalue["name"] = user_info->name;
	rtvalue["email"] = user_info->email;
	rtvalue["nick"] = user_info->nick;
	rtvalue["desc"] = user_info->desc;
	rtvalue["sex"] = user_info->sex;
	rtvalue["icon"] = user_info->icon;
    //更新到redis中
	Json::Value root;
	root["uid"] = user_info->uid;
	root["pwd"] = user_info->pwd;
	root["name"] = user_info->name;
	root["email"] = user_info->email;
	root["nick"] = user_info->nick;
	root["desc"] = user_info->desc;
	root["icon"] = user_info->icon;
	root["sex"] = user_info->sex;
	RedisMgr::getInstance()->Set(base_key, root.toStyledString());
}

bool LogicSystem::GetFriendApplyInfo(int to_uid, std::vector<std::shared_ptr<ApplyInfo>>& list)
{
    return MysqlMgr::getInstance()->GetFriendApplyInfo(to_uid, list, 0, 10);
}

bool LogicSystem::GetFriendList(int self_id, std::vector<std::shared_ptr<UserInfo>>& user_list)
{
	return MysqlMgr::getInstance()->GetFriendList(self_id, user_list);
}

bool LogicSystem::isPureDigit(const std::string& str)
{
	for (char c : str) {
		if (!isdigit(c)) {
			return false;
		}
	}
    return true;
}
