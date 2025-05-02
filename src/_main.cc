#include <iostream>
#include <boost/asio.hpp>
#include <csignal>
#include "logging.h"
#include "server.h"
#include "config_parser.h"
#include "config_manager.h"

int main(int argc, char *argv[])
{
    logging::init();
    BOOST_LOG_TRIVIAL(info) << "=== server start ===";

    try {
        if (argc != 2) {
            BOOST_LOG_TRIVIAL(fatal) << "Usage: webserver <path to config file>";
            return 1;
        }

        NginxConfigParser parser;
        NginxConfig config;
        if (!parser.Parse(argv[1], &config)) {
            BOOST_LOG_TRIVIAL(fatal) << "Failed to parse config file: " << argv[1];
            return 1;
        }

        ConfigManager cfg(config);
        int port = cfg.getPort();
        cfg.loadRoutes();

        boost::asio::io_service io;

        boost::asio::signal_set signals(io, SIGINT, SIGTERM);
        signals.async_wait([&](auto /*unused*/, auto /*unused*/) {
            BOOST_LOG_TRIVIAL(info) << "=== server shut-down ===";
            io.stop();
        });

        Server server(io, static_cast<short>(port), cfg);
        io.run();

    } catch (const std::exception& e) {
        BOOST_LOG_TRIVIAL(fatal) << "Exception: " << e.what();
        return 1;
    }
    return 0;
}
