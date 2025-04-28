#include "config_manager.h"

// production or local port config

ConfigManager::ConfigManager(const NginxConfig &config)
    : config_(config)
{
}

int ConfigManager::getPort() const
{
    for (const auto &statement : config_.statements_)
    {
        if (statement->tokens_.size() >= 2 && statement->tokens_[0] == "port")
        {
            int port = std::stoi(statement->tokens_[1]);
            if (port >= 0 && port <= 65535)
            {
                return port;
            }
            else
            {
                throw std::runtime_error("Invalid port number: " + statement->tokens_[1]);
            }
        }
        else
        {
            throw std::runtime_error("Invalid port statement: " + statement->ToString(0));
        }
    }
}
