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

        NginxConfigParser config_parser;
        NginxConfig config;
        if (!config_parser.Parse(argv[1], &config))
        {
            std::cerr << "Failed to parse config file: " << argv[1] << std::endl;
            return 1;
        }

        boost::asio::io_service io_service;

        // Use ConfigManager to extract port
        ConfigManager config_manager(config);
        int port = config_manager.getPort();

        // Create and run the server
        Server server(io_service, static_cast<short>(port));
        io_service.run();
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
