#include "LogicSystem.h"
#include "HttpConnection.h"
#include "VerifyGrpcClient.h"
#include "RedisMgr.h"
#include "MysqlMgr.h"

LogicSystem::LogicSystem()
{
    regGet("/get_test", [](std::shared_ptr<HttpConnection> conn) {
        beast::ostream(conn->_response.body()) << "receive get_test req" << std::endl;
        int i = 0;
        for (auto& elem : conn->_get_params) {
            i++;
            beast::ostream(conn->_response.body()) << "param" << i << " key is " << elem.first;
            beast::ostream(conn->_response.body()) << ", " << " value is " << elem.second << std::endl;
        }
    });
    regPost("/get_verifycode", [](std::shared_ptr<HttpConnection> conn) {
        auto body_str = boost::beast::buffers_to_string(conn->_request.body().data());
        std::cout << "receive body is " << body_str << std::endl;
        conn->_response.set(http::field::content_type, "text/json");
        Json::Value root;
        Json::Value src_root;
        Json::CharReaderBuilder readerBuilder;
        std::unique_ptr<Json::CharReader> reader(readerBuilder.newCharReader());
        std::string errs;
        bool parse_success = reader->parse(
            body_str.c_str(),
            body_str.c_str() + body_str.size(),
            &src_root,
            &errs
        );
        if (!parse_success || !src_root.isMember("email")) {
            std::cout << "Failed to parse JSON data! " << errs << std::endl;
            root["error"] = ErrorCodes::Error_Json;
            std::string jsonstr = root.toStyledString();
            beast::ostream(conn->_response.body()) << jsonstr;
            return;
        }
        auto email = src_root["email"].asString();
        GetVerifyRsp rsp = VerifyGrpcClient::getInstance()->GetVerifyCode(email);
        std::cout << "email is " << email << std::endl;
        root["error"] = 0;
        root["email"] = src_root["email"];
        std::string jsonstr = root.toStyledString();
        beast::ostream(conn->_response.body()) << jsonstr;
    });

    regPost("/user_register", [](std::shared_ptr<HttpConnection> connection) {
        auto body_str = boost::beast::buffers_to_string(connection->_request.body().data());
        std::cout << "receive body is " << body_str << std::endl;
        connection->_response.set(http::field::content_type, "text/json");
        Json::Value root;
        Json::Reader reader;
        Json::Value src_root;
        bool parse_success = reader.parse(body_str, src_root);
        if (!parse_success) {
            std::cout << "Failed to parse JSON data!" << std::endl;
            root["error"] = ErrorCodes::Error_Json;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->_response.body()) << jsonstr;
            return true;
        }
        //先查找redis中email对应的验证码是否合理
        std::string verify_code;
        bool b_get_verify = RedisMgr::getInstance()->Get(CODEPREFIX + src_root["email"].asString(), verify_code);
        if (!b_get_verify) {
            std::cout << " get verify code expired" << std::endl;
            root["error"] = ErrorCodes::VerifyExpired;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->_response.body()) << jsonstr;
            return true;
        }
        if (verify_code != src_root["verifycode"].asString()) {
            std::cout << " verify code error" << std::endl;
            root["error"] = ErrorCodes::VerifyCodeErr;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->_response.body()) << jsonstr;
            return true;
        }
        //访问数据库查找用户名是否存在
        int uid = MysqlMgr::getInstance()->RegUser(src_root["user"].asString(), src_root["email"].asString(), src_root["pwd"].asString());
        if (uid == 0 || uid == -1) {
            std::cout << "user or email exist" << std::endl;
            root["error"] = ErrorCodes::UserExist;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->_response.body()) << jsonstr;
            return true;
        }
        root["error"] = 0;
        root["email"] = src_root["email"];
        root["user"] = src_root["user"].asString();
        root["pwd"] = src_root["pwd"].asString();
        root["confirm"] = src_root["confirm"].asString();
        root["verifycode"] = src_root["verifycode"].asString();
        std::string jsonstr = root.toStyledString();
        beast::ostream(connection->_response.body()) << jsonstr;
        return true;
        });
}

bool LogicSystem::handleGet(std::string url, std::shared_ptr<HttpConnection> conn)
{
    if (_get_handlers.find(url) == _get_handlers.end()) {
        return false;
    }
    _get_handlers[url](conn);
    return true;
}

void LogicSystem::regGet(std::string url, HttpHandler handler)
{
    _get_handlers[url] = handler;
}

bool LogicSystem::handlePost(std::string url, std::shared_ptr<HttpConnection> conn)
{
    if (_post_handlers.find(url) == _post_handlers.end()) {
        return false;
    }
    _post_handlers[url](conn);
    return true;
}

void LogicSystem::regPost(std::string url, HttpHandler handler)
{
    _post_handlers[url] = handler;
}
