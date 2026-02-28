#include "ChatGrpcClient.h"
#include "RedisMgr.h"
#include "UserMgr.h"
#include "MysqlMgr.h"
#include "CSession.h"

ChatGrpcClient::ChatGrpcClient()
{
    auto& cfg = ConfigMgr::getInstance();
    auto server_list = cfg["PeerServer"]["Servers"];
    std::vector<std::string> words;
    std::stringstream ss(server_list);
    std::string word;
    while (std::getline(ss, word, ',')) {
        words.push_back(word);
    }
    for (auto& word : words) {
        if (cfg[word]["Name"].empty()) {
            continue;
        }
        _pools[cfg[word]["Name"]] = std::make_unique<ChatConPool>(5, cfg[word]["Host"], cfg[word]["Port"]);
    }
}

ChatGrpcClient::~ChatGrpcClient()
{
}

AddFriendRsp ChatGrpcClient::NotifyAddFriend(std::string server_ip, const AddFriendReq& req)
{
    AddFriendRsp rsp;
    return rsp;
}

AuthFriendRsp ChatGrpcClient::NotifyAuthFriend(std::string server_ip, const AuthFriendReq& req)
{
    AuthFriendRsp rsp;
    return rsp;
}

bool ChatGrpcClient::GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo)
{
    return false;
}

TextChatMsgRsp ChatGrpcClient::NotifyTextChatMsg(std::string server_ip, const TextChatMsgReq& req, const Json::Value& rtvalue)
{
    TextChatMsgRsp rsp;
    return rsp;
}
