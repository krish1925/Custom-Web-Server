#include "gtest/gtest.h"
#include "session.h"
#include <memory>
#include <boost/asio.hpp>

// Mock response handler for testing
class MockResponseHandler : public IResponseHandler {
public:
  std::string generateResponse(const std::string& request, bool& should_close) override {
    last_request = request;
    should_close = close_connection;
    return test_response;
  }
  
  std::string last_request;
  std::string test_response = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nHello";
  bool close_connection = false;
};

class SessionTest : public ::testing::Test {
protected:
  void SetUp() override {
    mock_handler = std::make_shared<MockResponseHandler>();
  }
  
  boost::asio::io_service io_service_;
  std::shared_ptr<MockResponseHandler> mock_handler;
};

// Test session creation
TEST_F(SessionTest, Constructor) {
  Session session(io_service_, mock_handler);
  EXPECT_TRUE(session.socket().is_open() == false);
}

// might need to add more tests for handle_read and handle_write, this will need more setup and possibly test subclasses