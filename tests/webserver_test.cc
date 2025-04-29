#include "gtest/gtest.h"
#include "config_parser.h"
#include <sstream>

// Define test fixture for the webserver tests
class WebServerTest : public ::testing::Test
{
protected:
  NginxConfigParser parser_;
  NginxConfig config_;
};

// Test to figure if the server can parse a simple port configuration
TEST_F(WebServerTest, SimplePortConfig)
{
  std::string config_str = "port 8080;";
  std::stringstream config_stream(config_str);
  bool success = parser_.Parse(&config_stream, &config_);
  EXPECT_TRUE(success);
  EXPECT_EQ(1, config_.statements_.size());
  EXPECT_EQ("port", config_.statements_[0]->tokens_[0]);
  EXPECT_EQ("8080", config_.statements_[0]->tokens_[1]);
}

// Test to figure out if the server correctly parses port within a block
TEST_F(WebServerTest, PortInBlock)
{
  std::string config_str =
      "server {\n"
      "  port 8080;\n"
      "}\n";
  std::stringstream config_stream(config_str);
  bool success = parser_.Parse(&config_stream, &config_);
  EXPECT_TRUE(success);
  EXPECT_EQ(1, config_.statements_.size());
  EXPECT_EQ("server", config_.statements_[0]->tokens_[0]);
  EXPECT_TRUE(config_.statements_[0]->child_block_.get() != nullptr);
  EXPECT_EQ(1, config_.statements_[0]->child_block_->statements_.size());
  EXPECT_EQ("port", config_.statements_[0]->child_block_->statements_[0]->tokens_[0]);
  EXPECT_EQ("8080", config_.statements_[0]->child_block_->statements_[0]->tokens_[1]);
}

// Test that the server can parse multiple config params
TEST_F(WebServerTest, MultipleParams)
{
  std::string config_str =
      "port 8080;\n"
      "server_name example.com;\n";
  std::stringstream config_stream(config_str);
  bool success = parser_.Parse(&config_stream, &config_);
  EXPECT_TRUE(success);
  EXPECT_EQ(2, config_.statements_.size());
  EXPECT_EQ("port", config_.statements_[0]->tokens_[0]);
  EXPECT_EQ("8080", config_.statements_[0]->tokens_[1]);
}

// Test that the server correctly parses nested config with port
TEST_F(WebServerTest, NestedConfigWithPort)
{
  std::string config_str =
      "http {\n"
      "  server {\n"
      "    port 8080;\n"
      "  }\n"
      "}\n";
  std::stringstream config_stream(config_str);
  bool success = parser_.Parse(&config_stream, &config_);
  EXPECT_TRUE(success);

  // Navigate to the nested port configuration
  EXPECT_EQ(1, config_.statements_.size());
  EXPECT_EQ("http", config_.statements_[0]->tokens_[0]);
  EXPECT_TRUE(config_.statements_[0]->child_block_.get() != nullptr);
  EXPECT_EQ(1, config_.statements_[0]->child_block_->statements_.size());
  EXPECT_EQ("server", config_.statements_[0]->child_block_->statements_[0]->tokens_[0]);
  EXPECT_TRUE(config_.statements_[0]->child_block_->statements_[0]->child_block_.get() != nullptr);
  EXPECT_EQ(1, config_.statements_[0]->child_block_->statements_[0]->child_block_->statements_.size());
  EXPECT_EQ("port", config_.statements_[0]->child_block_->statements_[0]->child_block_->statements_[0]->tokens_[0]);
  EXPECT_EQ("8080", config_.statements_[0]->child_block_->statements_[0]->child_block_->statements_[0]->tokens_[1]);
}

// Test that the server correctly handles comments in the config
TEST_F(WebServerTest, CommentsInConfig)
{
  std::string config_str =
      "# This is a comment\n"
      "port 8080; # This is an inline comment\n";
  std::stringstream config_stream(config_str);
  bool success = parser_.Parse(&config_stream, &config_);
  EXPECT_TRUE(success);
  EXPECT_EQ(1, config_.statements_.size());
  EXPECT_EQ("port", config_.statements_[0]->tokens_[0]);
  EXPECT_EQ("8080", config_.statements_[0]->tokens_[1]);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

// Test with: docker run --rm --entrypoint /usr/local/bin/webserver_test vibe-code-only:latest
