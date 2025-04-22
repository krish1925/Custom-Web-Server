#include "config_manager.h"
#include <iostream>

//production or local port config

const int ConfigManager::DEFAULT_PORT  = 8080;

ConfigManager::ConfigManager(const NginxConfig& config)
    : config_(config)
{
}

int ConfigManager::getPort() const
{
    for (const auto &statement : config_.statements_)
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
                    std::cerr << "invalid port number: " << port << std::endl;
                }
            }
            catch (const std::exception &e)
            {
                std::cerr << "invalid port number: " << statement->tokens_[1] << std::endl;
            }
        }
    }

    std::cout << "using default port " << DEFAULT_PORT << std::endl;
    return DEFAULT_PORT;
}
