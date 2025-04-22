#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include "config_parser.h"
#include <string>

// Class to manage configuration extraction and validation
class ConfigManager {
public:
    // Constructor takes a reference to a parsed NginxConfig
    ConfigManager(const NginxConfig& config);
    
    // Get port from config, with validation and default fallback
    int getPort() const;
    
    // Default port to use if not specified in config
    static const int DEFAULT_PORT;

private:
    // Reference to the parsed configuration
    const NginxConfig& config_;
};

#endif // CONFIG_MANAGER_H
