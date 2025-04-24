#include "gtest/gtest.h"
#include "config_manager.h"
#include "config_parser.h"
#include <sstream>

class ConfigManagerTest : public ::testing::Test
{
protected:
  NginxConfigParser parser_;
  NginxConfig config_;
};

// Test error when no port is specified
TEST_F(ConfigManagerTest, NoPort)
{
  std::string config_str = "server_name example.com;";
  std::stringstream config_stream(config_str);
  ASSERT_TRUE(parser_.Parse(&config_stream, &config_));
  ConfigManager config_manager(config_);

  EXPECT_THROW(config_manager.getPort(), std::runtime_error);
  try
  {
    config_manager.getPort();
  }
  catch (const std::runtime_error &e)
  {
    EXPECT_STREQ(e.what(), "Invalid port statement: server_name example.com;\n");
  }
}

// Test valid port extraction
TEST_F(ConfigManagerTest, ValidPort)
{
  std::string config_str = "port 9000;";
  std::stringstream config_stream(config_str);
  ASSERT_TRUE(parser_.Parse(&config_stream, &config_));
  ConfigManager config_manager(config_);

  EXPECT_EQ(9000, config_manager.getPort());
}

// Test invalid port number (negative)
TEST_F(ConfigManagerTest, InvalidNegativePort)
{
  std::string config_str = "port -80;";
  std::stringstream config_stream(config_str);

  ASSERT_TRUE(parser_.Parse(&config_stream, &config_));
  ConfigManager config_manager(config_);

  EXPECT_THROW(config_manager.getPort(), std::runtime_error);
  try
  {
    config_manager.getPort();
  }
  catch (const std::runtime_error &e)
  {
    EXPECT_STREQ(e.what(), "Invalid port number: -80");
  }
}

// Test invalid port number (large)
TEST_F(ConfigManagerTest, InvalidLargePort)
{
  std::string config_str = "port 70000;";
  std::stringstream config_stream(config_str);

  ASSERT_TRUE(parser_.Parse(&config_stream, &config_));
  ConfigManager config_manager(config_);

  EXPECT_THROW(config_manager.getPort(), std::runtime_error);
  // check if the exception message is "Invalid port number: 70000"
  try
  {
    config_manager.getPort();
  }
  catch (const std::runtime_error &e)
  {
    EXPECT_STREQ(e.what(), "Invalid port number: 70000");
  }
}

// Test invalid port format (non-numeric)
TEST_F(ConfigManagerTest, InvalidPortFormat)
{
  std::string config_str = "port abc;";
  std::stringstream config_stream(config_str);

  ASSERT_TRUE(parser_.Parse(&config_stream, &config_));
  ConfigManager config_manager(config_);

  EXPECT_THROW(config_manager.getPort(), std::invalid_argument);
  try
  {
    config_manager.getPort();
  }
  catch (const std::invalid_argument &e)
  {
    EXPECT_STREQ(e.what(), "stoi");
  }
}

// Test nested port configuration
TEST_F(ConfigManagerTest, NestedPort)
{
  std::string config_str =
      "server {\n"
      "  port 8888;\n"
      "}\n";
  std::stringstream config_stream(config_str);

  ASSERT_TRUE(parser_.Parse(&config_stream, &config_));
  ConfigManager config_manager(config_);

  // Currently ConfigManager doesn't search nested blocks
  EXPECT_THROW(config_manager.getPort(), std::runtime_error);
}

// Test port value 0 (edge of range)
TEST_F(ConfigManagerTest, ZeroPort)
{
  std::string config_str = "port 0;";
  std::stringstream config_stream(config_str);
  ASSERT_TRUE(parser_.Parse(&config_stream, &config_));
  ConfigManager config_manager(config_);
  // Should return default port
  EXPECT_EQ(0, config_manager.getPort());
}

// Test port value 65536 (invalid range)
TEST_F(ConfigManagerTest, InvalidTooLargePort)
{
  std::string config_str = "port 65536;";
  std::stringstream config_stream(config_str);
  ASSERT_TRUE(parser_.Parse(&config_stream, &config_));
  ConfigManager config_manager(config_);
  EXPECT_THROW(config_manager.getPort(), std::runtime_error);
  try
  {
    config_manager.getPort();
  }
  catch (const std::runtime_error &e)
  {
    EXPECT_STREQ(e.what(), "Invalid port number: 65536");
  }
}

// Test multiple port directives (should take the first one)
TEST_F(ConfigManagerTest, MultiplePortDirectives)
{
  std::string config_str = "port 9000; port 9001;";
  std::stringstream config_stream(config_str);
  ASSERT_TRUE(parser_.Parse(&config_stream, &config_));
  ConfigManager config_manager(config_);
  EXPECT_EQ(9000, config_manager.getPort());
}

// Test port directive with extra tokens (should still parse the number)
// Note: This depends on the parser allowing this, which the current one does.
TEST_F(ConfigManagerTest, PortDirectiveWithExtraTokens)
{
  std::string config_str = "port 8888 extra;";
  std::stringstream config_stream(config_str);
  ASSERT_TRUE(parser_.Parse(&config_stream, &config_));
  ConfigManager config_manager(config_);
  EXPECT_EQ(8888, config_manager.getPort());
}

// Test case where port value causes std::stoi exception (e.g., very large number string)
TEST_F(ConfigManagerTest, PortStoiException)
{
  // Use a number string longer than what std::stoi can handle
  std::string large_num_str(20, '9'); // "99999999999999999999"
  std::string config_str = "port " + large_num_str + ";";
  std::stringstream config_stream(config_str);

  ASSERT_TRUE(parser_.Parse(&config_stream, &config_));
  ConfigManager config_manager(config_);

  // Should catch exception
  EXPECT_THROW(config_manager.getPort(), std::out_of_range);
  try
  {
    config_manager.getPort();
  }
  catch (const std::out_of_range &e)
  {
    EXPECT_STREQ(e.what(), "stoi");
  }
}

TEST_F(ConfigManagerTest, EmptyConfig)
{
  std::string config_str = "";
  std::stringstream config_stream(config_str);
  ASSERT_FALSE(parser_.Parse(&config_stream, &config_));
}