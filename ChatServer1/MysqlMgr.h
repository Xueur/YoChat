#pragma once
#include "common.h"
#include "Singleton.h"
#include "MysqlDao.h"

class MysqlMgr : public Singleton<MysqlMgr>
{
    friend class Singleton<MysqlMgr>;
public:
    ~MysqlMgr();
    int RegUser(const std::string& name, const std::string& email, const std::string& pwd);
    bool CheckEmail(const std::string& name, const std::string& email);
    bool UpdatePwd(const std::string& email, const std::string& pwd);
    bool CheckPwd(const std::string& email, const std::string& pwd, UserInfo& userInfo);
    bool AddFriendApply(const int& from, const int& to);
    bool AuthFriendApply(const int& from, const int& to);
    bool AddFriend(int from, int to, const std::string& rmkname);
    bool GetFriendApplyInfo(int touid, std::vector<std::shared_ptr<ApplyInfo>>& applyList, int begin = 0, int limit = 10);
    bool GetFriendList(int self_id, std::vector<std::shared_ptr<UserInfo> >& user_info);
    std::shared_ptr<UserInfo> getUser(int uid);
    std::shared_ptr<UserInfo> getUser(const std::string& name);
    MysqlMgr();
    MysqlDao  _dao;
};

