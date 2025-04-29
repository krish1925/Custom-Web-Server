#include "config_manager.h"
#include "echo_handler.h"
#include "logging.h"
#include "static_file_handler.h"
#include <algorithm>
#include <stdexcept>
#include <boost/filesystem.hpp>

using boost::filesystem::path;

ConfigManager::ConfigManager(const NginxConfig &config)
    : config_(config)
{
    BOOST_LOG_TRIVIAL(info) << "[ConfigManager] Initialized";
}

int ConfigManager::getPort() const
{
    BOOST_LOG_TRIVIAL(debug) << "[ConfigManager] Reading port from config";
    for (const auto &statement : config_.statements_)
    {
        if (statement->tokens_.size() >= 2 && statement->tokens_[0] == "port")
        {
            int port = std::stoi(statement->tokens_[1]);
            if (port >= 0 && port <= 65535)
            {
                BOOST_LOG_TRIVIAL(info) << "[ConfigManager] Found port: " << port;
                return port;
            }
            else
            {
                BOOST_LOG_TRIVIAL(error) << "[ConfigManager] Invalid port number: " << statement->tokens_[1];
                throw std::runtime_error("Invalid port number: " + statement->tokens_[1]);
            }
        }
        else
        {
            BOOST_LOG_TRIVIAL(error) << "[ConfigManager] Invalid port statement: " << statement->ToString(0);
            throw std::runtime_error("Invalid port statement: " + statement->ToString(0));
        }
    }
}

void ConfigManager::loadRoutes()
{
    BOOST_LOG_TRIVIAL(debug) << "[ConfigManager] Loading routes";
    routes_.clear();

    for (const auto &st : config_.statements_)
    {
        if (st->tokens_.size() < 3 || st->tokens_[0] != "handler")
            BOOST_LOG_TRIVIAL(debug) << "[ConfigManager] Skipping directive: " << st->ToString(0);
            continue;

        std::string prefix = st->tokens_[1];
        std::string type = st->tokens_[2];
        BOOST_LOG_TRIVIAL(info) << "[ConfigManager] Processing handler directive: prefix=" << prefix << ", type=" << type;

        std::shared_ptr<IRequestHandler> h;
        if (type == "Echo")
        {
            h = std::make_shared<EchoHandler>();
            BOOST_LOG_TRIVIAL(info) << "[ConfigManager] Created EchoHandler for prefix " << prefix;
        }
        else if (type == "Static")
        {
            if (st->tokens_.size() < 4) {
                BOOST_LOG_TRIVIAL(error) << "[ConfigManager] Static handler missing root dir for prefix " << prefix;
                throw std::runtime_error("Static handler missing root dir");
            }
            std::string root = st->tokens_[3];
            h = std::make_shared<StaticFileHandler>(prefix, root);
            BOOST_LOG_TRIVIAL(info) << "[ConfigManager] Created StaticFileHandler for prefix " << prefix << " with root " << root;
        }
        else
        {
            BOOST_LOG_TRIVIAL(error) << "[ConfigManager] Unknown handler type: " << type;
            throw std::runtime_error("Unknown handler type: " + type);
        }
        routes_.push_back({prefix, h});
    }
    if (routes_.empty())
        BOOST_LOG_TRIVIAL(error) << "[ConfigManager] No handler directives found";
        throw std::runtime_error("No handler directives found");

    std::sort(routes_.begin(), routes_.end(),
              [](auto &a, auto &b)
              { return a.prefix.size() > b.prefix.size(); });
    BOOST_LOG_TRIVIAL(info) << "[ConfigManager] Loaded " << routes_.size() << " routes";
}

std::optional<std::shared_ptr<IRequestHandler>> ConfigManager::matchHandler(const std::string &uri) const
{
    BOOST_LOG_TRIVIAL(debug) << "[ConfigManager] Matching URI: " << uri;
    for (const auto &r : routes_) {
      if (uri.rfind(r.prefix, 0) == 0) {
        BOOST_LOG_TRIVIAL(info) << "[ConfigManager] URI " << uri << " matched prefix: " << r.prefix;
        return r.handler;
      }

    }
    BOOST_LOG_TRIVIAL(info) << "[ConfigManager] No handler match for URI: " << uri;
    return std::nullopt;
}

const std::vector<RouteEntry> &ConfigManager::routes() const
{
    BOOST_LOG_TRIVIAL(debug) << "[ConfigManager] Retrieving routes (" << routes_.size() << ")";
    return routes_;
}
