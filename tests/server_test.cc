#include "gtest/gtest.h"
#include "server.h"
#include <boost/asio.hpp>

class ServerTest : public ::testing::Test {
protected:
  boost::asio::io_service io_service_;
};

// Test server initialization with valid port
TEST_F(ServerTest, InitializationWithValidPort) {
  short port = 8080;
  EXPECT_NO_THROW(Server server(io_service_, port));
}