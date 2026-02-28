#pragma once
#include "Singleton.h"
#include "ConfigMgr.h"
#include <grpcpp/grpcpp.h> 
#include "message.grpc.pb.h"
#include "message.pb.h"
#include <queue>
#include <unordered_map>
#include "common.h"
#include "data.h"
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::AddFriendReq;
using message::AddFriendRsp;

using message::AuthFriendReq;
using message::AuthFriendRsp;

using message::GetChatServerRsp;
using message::LoginRsp;
using message::LoginReq;
using message::ChatService;

using message::TextChatMsgReq;
using message::TextChatMsgRsp;
using message::TextChatData;

class ChatConPool {
public:
    ChatConPool(size_t poolSize, std::string host, std::string port): 
        pool_size_(poolSize), host_(host), port_(port), b_stop_(false) {
        for (size_t i = 0; i < poolSize; ++i) {
            std::shared_ptr<Channel> channel = grpc::CreateChannel(host + ":" + port, grpc::InsecureChannelCredentials());
            connections_.push(ChatService::NewStub(channel));
        }
    }
    ~ChatConPool() {
        std::unique_lock<std::mutex> lock(mutex_);
        b_stop_ = true;
        cv_.notify_all();
    }
    std::shared_ptr< ChatService::Stub> getConnection() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] {return !connections_.empty() || b_stop_;});
        if (b_stop_) return nullptr;
        auto conn = std::move(connections_.front());
        connections_.pop();
        return conn;
    }
    void returnConnection(std::shared_ptr<ChatService::Stub> conn) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (b_stop_) return;
        connections_.push(std::move(conn));
        cv_.notify_one();
    }
private:
    bool b_stop_;
    size_t pool_size_;
    std::string host_;
    std::string port_;
    std::queue<std::shared_ptr<ChatService::Stub>> connections_;
    std::mutex mutex_;
    std::condition_variable cv_;
};

class ChatGrpcClient : public Singleton<ChatGrpcClient>
{
    friend class Singleton<ChatGrpcClient>;
public:
    ~ChatGrpcClient();
    AddFriendRsp NotifyAddFriend(std::string server_ip, const AddFriendReq& req);
    AuthFriendRsp NotifyAuthFriend(std::string server_ip, const AuthFriendReq& req);
    bool GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo);
    TextChatMsgRsp NotifyTextChatMsg(std::string server_ip, const TextChatMsgReq& req, const Json::Value& rtvalue);
private:
    ChatGrpcClient();
    std::unordered_map<std::string, std::unique_ptr<ChatConPool>> _pools;
};

