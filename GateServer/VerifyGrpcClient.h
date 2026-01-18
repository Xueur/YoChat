#pragma once
#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include "common.h"
#include "Singleton.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using message::VerifyService;
using message::GetVerifyReq;
using message::GetVerifyRsp;

class RPCConPool 
{
public:
    RPCConPool(size_t pool_size, std::string host, std::string port);
    ~RPCConPool();
    std::unique_ptr<VerifyService::Stub> getConn();
    void returnConn(std::unique_ptr<VerifyService::Stub> conn);
private:
    std::atomic<bool> b_stop_;
    size_t pool_size_;
    std::string host_;
    std::string port_;
    std::queue<std::unique_ptr<VerifyService::Stub>> connections_;
    std::condition_variable cv_;
    std::mutex mtx_;
};

class VerifyGrpcClient : public Singleton<VerifyGrpcClient>
{
    friend class Singleton<VerifyGrpcClient>;
public:
    GetVerifyRsp GetVerifyCode(const std::string email);
private:
    VerifyGrpcClient();
    std::unique_ptr<RPCConPool> pool_;
};

