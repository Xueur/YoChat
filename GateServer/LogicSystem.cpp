#include "LogicSystem.h"
#include "HttpConnection.h"
#include "VerifyGrpcClient.h"

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
