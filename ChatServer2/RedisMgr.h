#pragma once
#include "common.h"
#include <hiredis/hiredis.h>
#include "Singleton.h"
#include <queue>

class RedisConPool {
public:
    RedisConPool(int pool_size, const char* host, int port, const char* pwd);
    ~RedisConPool();
    redisContext* getConnection();
    void returnConnection(redisContext* con);
    void Close();
private:
    int _pool_size;
    const char* _host;
    int _port;
    const char* _pwd;
    std::queue<redisContext*> _connections;
    std::mutex _mtx;
    std::condition_variable _cv;
    std::atomic<bool> _stop;
};


class RedisMgr : public Singleton<RedisMgr>,
    public std::enable_shared_from_this<RedisMgr>
{
    friend class Singleton<RedisMgr>;
public:
    ~RedisMgr();
    bool Get(const std::string& key, std::string& value);
    bool Set(const std::string& key, const std::string& value);
    bool LPush(const std::string& key, const std::string& value);
    bool LPop(const std::string& key, std::string& value);
    bool RPush(const std::string& key, const std::string& value);
    bool RPop(const std::string& key, std::string& value);
    bool HSet(const std::string& key, const std::string& hkey, const std::string& value);
    bool HSet(const char* key, const char* hkey, const char* hvalue, size_t hvaluelen);
    std::string HGet(const std::string& key, const std::string& hkey);
    bool HDel(const std::string& key, const std::string& hkey);
    bool Del(const std::string& key);
    bool ExistsKey(const std::string& key);
private:
    RedisMgr();
    std::unique_ptr<RedisConPool> _con_pool;
};

