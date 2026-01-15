#include "Singleton.h"
#include "common.h"

class HttpConnection;
typedef std::function<void(std::shared_ptr<HttpConnection>)> HttpHandler;

class LogicSystem :public Singleton<LogicSystem>
{
    friend class Singleton<LogicSystem>;
public:
    ~LogicSystem() = default;
    bool handleGet(std::string, std::shared_ptr<HttpConnection>);
    void regGet(std::string, HttpHandler handler);
    bool handlePost(std::string, std::shared_ptr<HttpConnection>);
    void regPost(std::string, HttpHandler handler);

private:
    LogicSystem();
    std::unordered_map<std::string, HttpHandler> _post_handlers;
    std::unordered_map<std::string, HttpHandler> _get_handlers;
};
