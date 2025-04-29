#include <iostream>
#include <boost/asio.hpp>
#include "server.h"
#include "config_parser.h"
#include "config_manager.h"

int main(int argc, char *argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: webserver <path to config file>\n";
            return 1;
        }

        NginxConfigParser parser;
        NginxConfig config;
        if (!parser.Parse(argv[1], &config))
        {
            std::cerr << "Failed to parse config file: " << argv[1] << "\n";
            return 1;
        }

        ConfigManager cfg(config);
        int port = cfg.getPort();
        cfg.loadRoutes();

        boost::asio::io_service io;
        Server server(io, static_cast<short>(port), cfg);
        io.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
