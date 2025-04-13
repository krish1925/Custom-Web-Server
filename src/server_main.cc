#include <iostream>
#include <boost/asio.hpp>
#include "server.h"
#include "config_parser.h"

// Extract port from config and add error checking
int getPortFromConfig(const NginxConfig &config)
{
    for (const auto &statement : config.statements_)
    {
        if (statement->tokens_.size() >= 2 && statement->tokens_[0] == "port")
        {
            try
            {
                int port = std::stoi(statement->tokens_[1]);
                if (port > 0 && port < 65536)
                {
                    return port;
                }
                else
                {
                    std::cerr << "Invalid port number: " << port << std::endl;
                }
            }
            catch (const std::exception &e)
            {
                std::cerr << "Invalid port number: " << statement->tokens_[1] << std::endl;
            }
        }
    }

    std::cout << "Using default port 8080" << std::endl;
    return 8080;
}

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

        int port = getPortFromConfig(config);
        Server server(io_service, static_cast<short>(port));

        io_service.run();
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}