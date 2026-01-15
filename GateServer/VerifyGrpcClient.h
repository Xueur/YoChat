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

class VerifyGrpcClient : public Singleton<VerifyGrpcClient>
{
    friend class Singleton<VerifyGrpcClient>;
public:
    GetVerifyRsp GetVerifyCode(const std::string email) {
        GetVerifyRsp response;
        GetVerifyReq request;
        ClientContext context;
        request.set_email(email);
        Status status = stub_->GetVerifyCode(&context, request, &response);
        if (!status.ok()) response.set_error(ErrorCodes::RPCFailed);
        return response;
    }
private:
    VerifyGrpcClient() {
        std::shared_ptr<Channel> channel = grpc::CreateChannel("127.0.0.1:50051", grpc::InsecureChannelCredentials());
        stub_ = VerifyService::NewStub(channel);
    }
    std::unique_ptr<VerifyService::Stub> stub_;
};

