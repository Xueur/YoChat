#pragma once
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include <boost/beast/http.hpp>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <functional>
#include <map>
#include <memory>
#include <iostream>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp

enum ErrorCodes {
    Success = 0,
    Error_Json = 1001,  //json½âÎö´íÎó
    RPCFailed = 1002,  //RPCÇëÇó´íÎó
};
