#include "UserMgr.h"

std::shared_ptr<CSession> UserMgr::GetSession(int uid)
{
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _uid_to_sessions.find(uid);
    if (it != _uid_to_sessions.end()) {
        return it->second;
    }
    return nullptr;
}

void UserMgr::SetUserSession(int uid, std::shared_ptr<CSession> session)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _uid_to_sessions[uid] = session;
}

void UserMgr::RmvUserSession(int uid)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _uid_to_sessions.erase(uid);
}

UserMgr::UserMgr()
{

}

UserMgr::~UserMgr()
{
    _uid_to_sessions.clear();
}