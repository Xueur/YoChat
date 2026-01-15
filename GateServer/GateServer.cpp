#include "common.h"
#include "CServer.h"
#include "ConfigMgr.h"
	
int main()
{
    ConfigMgr gCfgMgr;
    std::string gate_port_str = gCfgMgr["GateServer"]["port"];
    unsigned short gate_port = atoi(gate_port_str.c_str());
    try {
        unsigned short port = 8080;
        net::io_context ioc{ 4 };
        net::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&](beast::error_code const& ec, int signal_num)
            {
                if (ec) return;
                ioc.stop();
            });
        std::make_shared<CServer>(ioc, port)->start();
        ioc.run();
    } catch (std::exception& exp) {
        std::cout << "exception is " << exp.what() << std::endl;
        return EXIT_FAILURE;
    }
    
}