#define BOOST_BIND_GLOBAL_PLACEHOLDERS // for boost::bind
#include <gtest/gtest.h>
#include <boost/asio.hpp>
#include <thread>
#include <chrono>
#include <sstream>
#include <filesystem>
#include <fstream>

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
    Server server(io_service, static_cast<short>(mgr.getPort()), mgr);
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

TEST(Integration, StaticFileHandler)
{
    // Create a test directory and file
    std::string test_dir = "/tmp/static_test_files";
    std::filesystem::create_directories(test_dir);
    std::string test_file_path = test_dir + "/index.html";
    std::string test_content = "<html><body>Static file test content</body></html>";

    // Create the test file
    {
        std::ofstream file(test_file_path);
        file << test_content;
    }

    // Configure the server with a static file handler
    std::stringstream ss;
    ss << "port " << kTestPort << ";\n"
       << "handler /echo Echo;\n"
       << "handler /static Static " << test_dir << ";";

    NginxConfigParser parser;
    NginxConfig config;
    ASSERT_TRUE(parser.Parse(&ss, &config));

    // Start the server
    boost::asio::io_service io_service;
    ConfigManager mgr(config);
    mgr.loadRoutes();
    std::thread srv([&io_service, &mgr]()
                    {
        Server server(io_service, static_cast<short>(mgr.getPort()), mgr);
        io_service.run(); });

    // Give it a moment to bind/listen
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Now exercise it as a client
    boost::asio::io_service client_io;
    tcp::socket sock(client_io);
    sock.connect({boost::asio::ip::address::from_string("127.0.0.1"), kTestPort});

    // Request the static file
    std::string req = "GET /static/index.html HTTP/1.1\r\nHost: localhost\r\n\r\n";
    boost::asio::write(sock, boost::asio::buffer(req));

    // Read back response
    std::array<char, 8192> buf;
    size_t n = sock.read_some(boost::asio::buffer(buf));
    std::string resp(buf.data(), n);

    // It should contain a 200 OK and the file content
    EXPECT_NE(resp.find("HTTP/1.1 200 OK"), std::string::npos);
    EXPECT_NE(resp.find(test_content), std::string::npos) << "Response: " << resp;

    // Test non-existent file
    std::string req_not_found = "GET /static/not_found.html HTTP/1.1\r\nHost: localhost\r\n\r\n";
    boost::asio::write(sock, boost::asio::buffer(req_not_found));

    n = sock.read_some(boost::asio::buffer(buf));
    resp = std::string(buf.data(), n);

    // It should contain a 404 Not Found
    EXPECT_NE(resp.find("HTTP/1.1 404 Not Found"), std::string::npos) << "Response: " << resp;

    // Clean up
    sock.close();
    io_service.stop();
    srv.join();

    // Remove test files
    std::filesystem::remove_all(test_dir);
}