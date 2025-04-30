#define BOOST_BIND_GLOBAL_PLACEHOLDERS // Fix boost bind placeholders warning

#include "gtest/gtest.h"
#include "session.h"
#include "echo_handler.h"

class MockConfigManager : public ConfigManager
{
public:
    MockConfigManager(const NginxConfig &config, std::shared_ptr<IRequestHandler> handler)
        : ConfigManager(config), mock_handler_(handler) {}

    std::optional<std::shared_ptr<IRequestHandler>> matchHandler(const std::string &uri) const override
    {
        return mock_handler_;
    }

private:
    std::shared_ptr<IRequestHandler> mock_handler_;
};

class MockRequestHandler : public IRequestHandler
{
public:
    MockRequestHandler() : close_connection(false) {}

    Response handle(const Request &req, bool &should_close) override
    {
        last_request = req;
        should_close = close_connection;
        return my_response;
    }

    Request last_request;
    bool close_connection;
    Response my_response;
};

class SessionTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        mock_handler = std::make_shared<MockRequestHandler>();
        // Use MockConfigManager instead of regular ConfigManager
        mock_config_manager_ = std::make_unique<MockConfigManager>(default_config_, mock_handler);
    }

    NginxConfig default_config_; // Empty config for testing
    boost::asio::io_service io_service_;
    std::shared_ptr<MockRequestHandler> mock_handler; // Mock response handler for testing
    std::unique_ptr<MockConfigManager> mock_config_manager_;
};

// Test session creation
TEST_F(SessionTest, Constructor)
{
    Session session(io_service_, *mock_config_manager_);
    EXPECT_TRUE(session.socket().is_open() == false);
}

class TestableSession : public Session
{
public:
    // Expose protected methods for testing
    using Session::buffer_;
    using Session::handle_read;
    using Session::handle_write;
    using Session::Session;

    // Add accessor for data_
    char *get_data() { return data_; }

    // Method to set request buffer directly for testing
    void setRequestBuffer(const std::string &buffer)
    {
        buffer_ = buffer;
    }

protected:
    // Override destroy to prevent "delete this" in tests
    void destroy() override
    {
        // Do nothing in tests since we're stack-allocated
    }
};

// HandleReadComplete test
TEST_F(SessionTest, HandleReadComplete)
{
    TestableSession session(io_service_, *mock_config_manager_);

    // Set up request buffer with a complete HTTP request
    std::string complete_request = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
    std::copy(complete_request.begin(), complete_request.end(), session.get_data());

    // Call handle_read directly
    boost::system::error_code no_error;
    session.handle_read(no_error, complete_request.length());

    // Verify the request was processed
    EXPECT_EQ(complete_request, mock_handler->last_request.raw);
}

// HandleReadPartial test
TEST_F(SessionTest, HandleReadPartial)
{
    TestableSession session(io_service_, *mock_config_manager_);

    // Set up request buffer with a partial HTTP request
    std::string partial_request = "GET / HTTP/1.1\r\nHost: localhost\r\n";
    std::copy(partial_request.begin(), partial_request.end(), session.get_data());

    // Call handle_read directly
    boost::system::error_code no_error;
    session.handle_read(no_error, partial_request.length());

    // Verify the request was stored but not processed
    EXPECT_TRUE(mock_handler->last_request.raw.empty());
}

// Test handle_read with an error
TEST_F(SessionTest, HandleReadError)
{
    TestableSession session(io_service_, *mock_config_manager_);
    boost::system::error_code ec = boost::asio::error::connection_reset;

    EXPECT_NO_THROW(session.handle_read(ec, 0));
    EXPECT_TRUE(mock_handler->last_request.raw.empty());
}

// Test handle_write with an error
TEST_F(SessionTest, HandleWriteError)
{
    TestableSession session(io_service_, *mock_config_manager_);
    boost::system::error_code ec = boost::asio::error::connection_reset;

    EXPECT_NO_THROW(session.handle_write(ec));
}

// Test handle_write success with Connection: keep-alive (default)
TEST_F(SessionTest, HandleWriteKeepAlive)
{
    TestableSession session(io_service_, *mock_config_manager_);
    mock_handler->close_connection = false;

    std::string req = "GET / HTTP/1.1\r\nHost: test\r\n\r\n";
    std::copy(req.begin(), req.end(), session.get_data());
    session.handle_read(boost::system::error_code(), req.length());

    boost::system::error_code no_error;
    session.handle_write(no_error);

    EXPECT_TRUE(session.buffer_.empty());
}

// Test handle_write success with Connection: close
TEST_F(SessionTest, HandleWriteCloseConnection)
{
    TestableSession session(io_service_, *mock_config_manager_);
    mock_handler->close_connection = true;

    std::string req = "GET / HTTP/1.1\r\nHost: test\r\nConnection: close\r\n\r\n";
    std::copy(req.begin(), req.end(), session.get_data());
    session.handle_read(boost::system::error_code(), req.length());

    boost::system::error_code no_error;
    EXPECT_NO_THROW(session.handle_write(no_error));
}

// Test EchoHandler directly
TEST(EchoHandlerTest, GenerateResponseKeepAlive)
{
    EchoHandler handler;
    Request request;
    request.raw = "GET / HTTP/1.1\r\nHost: test\r\n\r\n";
    bool should_close = true;
    Response res = handler.handle(request, should_close);
    std::string response = res.to_string();

    EXPECT_FALSE(should_close);
    EXPECT_NE(response.find("HTTP/1.1 200 OK"), std::string::npos);
    EXPECT_NE(response.find("Content-Type: text/plain"), std::string::npos);
    EXPECT_NE(response.find("Connection: keep-alive"), std::string::npos);
    EXPECT_NE(response.find("Content-Length: " + std::to_string(request.raw.length())), std::string::npos);
    EXPECT_NE(response.find("\r\n\r\n" + request.raw), std::string::npos);
}

TEST(EchoHandlerTest, GenerateResponseClose)
{
    EchoHandler handler;
    Request request;
    request.raw = "GET / HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n";
    bool should_close = false;
    Response res = handler.handle(request, should_close);
    std::string response = res.to_string();

    EXPECT_TRUE(should_close);
    EXPECT_NE(response.find("HTTP/1.1 200 OK"), std::string::npos);
    EXPECT_NE(response.find("Content-Type: text/plain"), std::string::npos);
    EXPECT_NE(response.find("Connection: close"), std::string::npos);
    EXPECT_NE(response.find("Content-Length: " + std::to_string(request.raw.length())), std::string::npos);
    EXPECT_NE(response.find("\r\n\r\n" + request.raw), std::string::npos);
}

// Test multiple reads building up the request
TEST_F(SessionTest, HandleReadMultiplePartial)
{
    TestableSession session(io_service_, *mock_config_manager_);

    std::string part1 = "GET / HTTP/1.1\r\n";
    std::string part2 = "Host: example.com\r\n\r\n";

    boost::system::error_code no_error;

    std::copy(part1.begin(), part1.end(), session.get_data());
    session.handle_read(no_error, part1.length());
    EXPECT_TRUE(mock_handler->last_request.raw.empty());
    EXPECT_EQ(session.buffer_, part1);

    std::copy(part2.begin(), part2.end(), session.get_data());
    session.handle_read(no_error, part2.length());
    EXPECT_FALSE(mock_handler->last_request.raw.empty());
    EXPECT_EQ(mock_handler->last_request.raw, part1 + part2);
}
