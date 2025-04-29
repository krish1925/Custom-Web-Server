#include "config_manager.h"
#include "request_handler.h"
#include "echo_handler.h"
#include "static_file_handler.h"
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <boost/filesystem.hpp>

using boost::filesystem::path;

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

void ConfigManager::loadRoutes()
{
    routes_.clear();

    for (const auto& st : config_.statements_) {
        if (st->tokens_.size() < 3 || st->tokens_[0] != "handler") continue;

        std::string prefix = st->tokens_[1];
        std::string type   = st->tokens_[2];

        std::shared_ptr<IRequestHandler> h;
        if (type == "Echo") {
            h = std::make_shared<EchoHandler>();
        } else if (type == "Static") {
            if (st->tokens_.size() < 4)
                throw std::runtime_error("Static handler missing root dir");
            std::string root = st->tokens_[3];
            h = std::make_shared<StaticFileHandler>(prefix, root);
        } else {
            throw std::runtime_error("Unknown handler type: " + type);
        }
        routes_.push_back({prefix, h});
    }
    if (routes_.empty())
        throw std::runtime_error("No handler directives found");

    std::sort(routes_.begin(), routes_.end(),
              [](auto& a, auto& b){ return a.prefix.size() > b.prefix.size(); });
}

std::optional<std::shared_ptr<IRequestHandler>>
ConfigManager::matchHandler(const std::string& uri) const
{
    for (const auto& r : routes_)
        if (uri.rfind(r.prefix, 0) == 0) return r.handler;
    std::cout << "No Match" << std::endl;
    return std::nullopt;
}
