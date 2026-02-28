#pragma once
#include "Singleton.h"
#include <unordered_map>
#include <memory>
#include <mutex>

class CSession;
class UserMgr: public Singleton<UserMgr>
{
    friend class Singleton<UserMgr>;
public:
    ~UserMgr();
    std::shared_ptr<CSession> GetSession(int uid);
    void SetUserSession(int uid, std::shared_ptr<CSession> session);
    void RmvUserSession(int uid);
private:
    UserMgr();
    std::unordered_map<int, std::shared_ptr<CSession>> _uid_to_sessions;
    std::mutex _mutex;
};

