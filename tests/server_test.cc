#include "gtest/gtest.h"
#include "server.h"
#include <boost/system/error_code.hpp>

// Mock Session class to verify start() is called
class MockSession : public Session
{
public:
    MockSession(boost::asio::io_service &io_service, ConfigManager &cfg)
        : Session(io_service, cfg) {}

    // Need to override socket() to prevent pure virtual function call
    tcp::socket &socket() override { return Session::socket(); }
};

// Test subclass of Server that allows us to:
// 1. Access protected handle_accept method
// 2. Override start_accept to verify it's called
class TestableServer2 : public Server
{
public:
    TestableServer2(boost::asio::io_service &io_service, short port, ConfigManager &cfg)
        : Server(io_service, port, cfg), start_accept_called(false) {}

    using Server::handle_accept;

    void start_accept() override
    {
        start_accept_called = true;
    }

    bool start_accept_called;
};

class MockResponseHandlerForServer : public IResponseHandler
{
public:
    std::string generateResponse(const std::string &request, bool &should_close) override
    {
        should_close = false;
        return "mock response";
    }
};

class TestableServer : public Server
{
public:
    using Server::handle_accept;
    using Server::Server;
    using Server::start_accept;

    bool error_path_taken = false;

    void handle_accept(Session *new_session, const boost::system::error_code &error) override
    {
        if (!error)
        {
            error_path_taken = false;
            delete new_session;
        }
        else
        {
            error_path_taken = true;
            delete new_session;
        }
    }
};

class ServerTest : public ::testing::Test
{
protected:
    boost::asio::io_service io_service_;
    std::shared_ptr<MockResponseHandlerForServer> mock_handler_; // Mock response handler for testing
    std::unique_ptr<ConfigManager> config_manager_;              // Dummy config manager for testing
    NginxConfig default_config_;                                 // Empty config for testing

    void SetUp() override
    {
        mock_handler_ = std::make_shared<MockResponseHandlerForServer>();
        config_manager_ = std::make_unique<ConfigManager>(default_config_);
    }
};

// Test server initialization with valid port
TEST_F(ServerTest, InitializationWithValidPort)
{
    short port = 8080;
    EXPECT_NO_THROW(Server server(io_service_, port, *config_manager_));
}

TEST_F(ServerTest, HandleAcceptErrorPath)
{
    short port = 8081;
    TestableServer test_server(io_service_, port, mock_handler_);

    Session *dummy_session = new Session(io_service_, *config_manager_, mock_handler_); // Create dummy session for testing
    boost::system::error_code ec = boost::asio::error::operation_aborted;

    test_server.handle_accept(dummy_session, ec); // Call handle_accept with dummy session and error code

    EXPECT_TRUE(test_server.error_path_taken); // Verify that error path was taken as expected
}

TEST_F(ServerTest, HandleAcceptWithoutError)
{
    boost::asio::io_service io_service;
    TestableServer2 server(io_service, 8080, *config_manager_);

    MockSession *session = new MockSession(io_service, *config_manager_);

    boost::system::error_code no_error;
    server.handle_accept(session, no_error);

    EXPECT_TRUE(server.start_accept_called);
}

TEST_F(ServerTest, HandleAcceptWithError)
{
    boost::asio::io_service io_service;
    TestableServer2 server(io_service, 8080, *config_manager_);

    MockSession *session = new MockSession(io_service, *config_manager_);

    // Call handle_accept with an error
    boost::system::error_code error = boost::asio::error::connection_refused;

    // We can't directly verify session was deleted,
    // but memory leak detectors will catch it if it wasn't
    server.handle_accept(session, error);

    // Verify start_accept was called even with the error
    EXPECT_TRUE(server.start_accept_called);
}
