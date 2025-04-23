#include "gtest/gtest.h"
#include "server.h"
#include "session.h"
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <memory>

class MockResponseHandlerForServer : public IResponseHandler {
public:
    std::string generateResponse(const std::string& request, bool& should_close) override {
        should_close = false;
        return "mock response";
    }
};

class TestableServer : public Server {
public:
    using Server::Server;
    using Server::handle_accept;
    using Server::start_accept;

    bool error_path_taken = false;

    void handle_accept(Session *new_session, const boost::system::error_code &error) override {
        if (!error) {
            error_path_taken = false;
            delete new_session;
        }
        else {
            error_path_taken = true;
            delete new_session;
        }
    }
};

class ServerTest : public ::testing::Test {
protected:
    boost::asio::io_service io_service_; 
    std::shared_ptr<MockResponseHandlerForServer> mock_handler_; // Mock response handler for testing

    void SetUp() override {
        mock_handler_ = std::make_shared<MockResponseHandlerForServer>();
    }
};

// Test server initialization with valid port
TEST_F(ServerTest, InitializationWithValidPort) {
    short port = 8080;
    EXPECT_NO_THROW(Server server(io_service_, port));
}

TEST_F(ServerTest, HandleAcceptErrorPath) {
    short port = 8081; 
    TestableServer test_server(io_service_, port, mock_handler_); 

    Session* dummy_session = new Session(io_service_, mock_handler_); // Create dummy session for testing
    boost::system::error_code ec = boost::asio::error::operation_aborted; 

    test_server.handle_accept(dummy_session, ec); // Call handle_accept with dummy session and error code

    EXPECT_TRUE(test_server.error_path_taken); // Verify that error path was taken as expected
}
