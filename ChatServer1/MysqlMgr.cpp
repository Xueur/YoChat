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
std::shared_ptr<UserInfo> MysqlMgr::getUser(int uid) {
    return _dao.getUser(uid);
}
MysqlMgr::MysqlMgr() {
}
