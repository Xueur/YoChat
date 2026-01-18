#include "VerifyGrpcClient.h"
#include "ConfigMgr.h"

GetVerifyRsp VerifyGrpcClient::GetVerifyCode(const std::string email) {
    GetVerifyRsp response;
    GetVerifyReq request;
    ClientContext context;
    request.set_email(email);
    auto stub = pool_->getConn();
    Status status = stub->GetVerifyCode(&context, request, &response);
    if (!status.ok()) response.set_error(ErrorCodes::RPCFailed);
    pool_->returnConn(std::move(stub));
    return response;
}

VerifyGrpcClient::VerifyGrpcClient() {
    auto& config = ConfigMgr::getInstance();
    std::string host = config["VerifyServer"]["Host"];   
    std::cout << host << std::endl;
    std::string port = config["VerifyServer"]["Port"];
    std::cout << port << std::endl;
    pool_.reset(new RPCConPool(5, host, port));
}

RPCConPool::RPCConPool(size_t pool_size, std::string host, std::string port):
    b_stop_(false), pool_size_(pool_size), host_(host), port_(port)
{
    for (int i = 0; i < pool_size_; ++i) {
        std::shared_ptr<Channel> channel = grpc::CreateChannel(host_ + ":" + port_, grpc::InsecureChannelCredentials());
        connections_.push(VerifyService::NewStub(channel));
        //临时右值，push的优化，不需要std::move
    }
}

RPCConPool::~RPCConPool() {
    std::unique_lock<std::mutex> lock(mtx_);
    b_stop_ = true;
    cv_.notify_all();
    while (!connections_.empty()) {
        connections_.pop();
    }
}

std::unique_ptr<VerifyService::Stub> RPCConPool::getConn() {
    std::unique_lock<std::mutex> lock(mtx_);
    cv_.wait(lock, [this]() { return b_stop_ || !connections_.empty();});
    if (b_stop_) {
        return nullptr;
    }
    auto conn = std::move(connections_.front());
    connections_.pop();
    return conn;
}

void RPCConPool::returnConn(std::unique_ptr<VerifyService::Stub> conn) {
    std::unique_lock<std::mutex> lock(mtx_);
    if (b_stop_) {
        return;
    }
    connections_.push(std::move(conn));
}