#pragma once
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include <boost/beast/http.hpp>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <functional>
#include <map>
#include <queue>
#include <memory>
#include <iostream>
#include <thread>
#include <atomic>
#include <cassert>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp

enum ErrorCodes {
    Success = 0,
    Error_Json = 1001,  //json解析错误
    RPCFailed = 1002,  //RPC请求错误
    VerifyExpired = 1003, //验证码过期
    VerifyCodeErr = 1004, //验证码错误
    UserExist = 1005, //用户已经存在
};

class Defer {
public:
	// 接受一个lambda表达式或者函数指针
	Defer(std::function<void()> func) : func_(func) {}

	// 析构函数中执行传入的函数
	~Defer() {
		func_();
	}

private:
	std::function<void()> func_;
};


#define CODEPREFIX "code_"