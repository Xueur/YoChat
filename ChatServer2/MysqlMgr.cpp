#include "MysqlMgr.h"

MysqlMgr::~MysqlMgr() {
}

int MysqlMgr::RegUser(const std::string& name, const std::string& email, const std::string& pwd)
{
    return _dao.RegUser(name, email, pwd);
}

bool MysqlMgr::CheckEmail(const std::string& name, const std::string& email) {
    return _dao.CheckEmail(name, email);
}
bool MysqlMgr::UpdatePwd(const std::string& email, const std::string& pwd) {
    return _dao.UpdatePwd(email, pwd);
}
bool MysqlMgr::CheckPwd(const std::string& email, const std::string& pwd, UserInfo& userInfo) {
    return _dao.CheckPwd(email, pwd, userInfo);
}
bool MysqlMgr::AddFriendApply(const int& from, const int& to)
{
    return _dao.AddFriendApply(from, to);
}
bool MysqlMgr::AuthFriendApply(const int& from, const int& to)
{
    return _dao.AuthFriendApply(from, to);
}
bool MysqlMgr::AddFriend(int from, int to, const std::string& rmkname)
{
    return _dao.AddFriend(from, to, rmkname);
}
bool MysqlMgr::GetFriendApplyInfo(int touid, std::vector<std::shared_ptr<ApplyInfo>>& applyList, int begin, int limit)
{
    return _dao.GetFriendApplyInfo(touid, applyList, begin, limit);
}
bool MysqlMgr::GetFriendList(int self_id, std::vector<std::shared_ptr<UserInfo>>& user_info)
{
    return _dao.GetFriendList(self_id, user_info);
}
std::shared_ptr<UserInfo> MysqlMgr::getUser(int uid) {
    return _dao.getUser(uid);
}
std::shared_ptr<UserInfo> MysqlMgr::getUser(const std::string& name)
{
    return _dao.getUser(name);
}
MysqlMgr::MysqlMgr() {
}
