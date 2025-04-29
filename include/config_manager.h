#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include "config_parser.h"
#include "request_handler.h"
#include "static_file_handler.h"
#include "echo_handler.h"
#include <memory>
#include <optional>
#include <string>
#include <vector>

struct RouteEntry {
    std::string prefix;
    std::shared_ptr<IRequestHandler> handler;
};

class ConfigManager {
public:
    explicit ConfigManager(const NginxConfig& config);

    int getPort() const;
    void loadRoutes();
    std::optional<std::shared_ptr<IRequestHandler>>
    matchHandler(const std::string& uri) const;

    const std::vector<RouteEntry>& routes() const { return routes_; }

private:
    const NginxConfig&   config_;
    std::vector<RouteEntry> routes_;
};

#endif

