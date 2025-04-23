#define BOOST_BIND_GLOBAL_PLACEHOLDERS  // for boost::bind
#include <gtest/gtest.h>
#include <boost/asio.hpp>
#include <thread>
#include <chrono>
#include <sstream>

#include "server.h"
#include "config_parser.h"
#include "config_manager.h"

using boost::asio::ip::tcp;

// choose a port unlikely to clash
static const short kTestPort = 9009;

// This helper spins up your server in its own thread.
void runTestServer()
{
    // parse a one‑line config that sets our test port
    std::stringstream ss;
    ss << "port " << kTestPort << ";";
    NginxConfigParser parser;
    NginxConfig config;
    ASSERT_TRUE(parser.Parse(&ss, &config));

    boost::asio::io_service io_service;
    ConfigManager mgr(config);
    Server server(io_service, static_cast<short>(mgr.getPort()));
    io_service.run();
}

TEST(Integration, EchoEndpoint)
{
    // start the server
    std::thread srv(runTestServer);
    // give it a moment to bind/listen
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // now exercise it as a client
    boost::asio::io_service client_io;
    tcp::socket sock(client_io);
    sock.connect({boost::asio::ip::address::from_string("127.0.0.1"), kTestPort});

    std::string req = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
    boost::asio::write(sock, boost::asio::buffer(req));

    // read back
    std::array<char, 8192> buf;
    size_t n = sock.read_some(boost::asio::buffer(buf));
    std::string resp(buf.data(), n);

    // it should contain a 200 OK and echo our request
    EXPECT_NE(resp.find("HTTP/1.1 200 OK"), std::string::npos);
    EXPECT_NE(resp.find(req), std::string::npos);

    // clean up
    sock.close();
    // we don't have a clean shutdown hook—detach is ok since process exits
    srv.detach();
}