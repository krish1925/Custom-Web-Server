#include "gtest/gtest.h"
#include "config_manager.h"
#include "config_parser.h"
#include <sstream>

class ConfigManagerTest : public ::testing::Test {
protected:
  NginxConfigParser parser_;
  NginxConfig config_;
};

// Test default port when no port is specified
TEST_F(ConfigManagerTest, DefaultPort) {
  std::string config_str = "server_name example.com;";
  std::stringstream config_stream(config_str);
  ASSERT_TRUE(parser_.Parse(&config_stream, &config_));
  ConfigManager config_manager(config_);
  
  EXPECT_EQ(ConfigManager::DEFAULT_PORT, config_manager.getPort());
}

// Test valid port extraction
TEST_F(ConfigManagerTest, ValidPort) {
  std::string config_str = "port 9000;";
  std::stringstream config_stream(config_str);
  ASSERT_TRUE(parser_.Parse(&config_stream, &config_));
  ConfigManager config_manager(config_);
  
  EXPECT_EQ(9000, config_manager.getPort());
}

// Test invalid port number (negative)
TEST_F(ConfigManagerTest, InvalidNegativePort) {
  std::string config_str = "port -80;";
  std::stringstream config_stream(config_str);
  
  ASSERT_TRUE(parser_.Parse(&config_stream, &config_));
  ConfigManager config_manager(config_);
  
  // should return default port
  EXPECT_EQ(ConfigManager::DEFAULT_PORT, config_manager.getPort());
}

// Test invalid port number (large)
TEST_F(ConfigManagerTest, InvalidLargePort) {
  std::string config_str = "port 70000;";
  std::stringstream config_stream(config_str);
  
  ASSERT_TRUE(parser_.Parse(&config_stream, &config_));
  ConfigManager config_manager(config_);
  
  // should return default port
  EXPECT_EQ(ConfigManager::DEFAULT_PORT, config_manager.getPort());
}

// Test invalid port format (non-numeric)
TEST_F(ConfigManagerTest, InvalidPortFormat) {
  std::string config_str = "port abc;";
  std::stringstream config_stream(config_str);
  
  ASSERT_TRUE(parser_.Parse(&config_stream, &config_));
  ConfigManager config_manager(config_);
  
  // Should return default port
  EXPECT_EQ(ConfigManager::DEFAULT_PORT, config_manager.getPort());
}

// Test nested port configuration
TEST_F(ConfigManagerTest, NestedPort) {
  std::string config_str = 
    "server {\n"
    "  port 8888;\n"
    "}\n";
  std::stringstream config_stream(config_str);
  
  ASSERT_TRUE(parser_.Parse(&config_stream, &config_));
  ConfigManager config_manager(config_);
  
  // Currently ConfigManager doesn't search nested blocks so this should return the default port
  EXPECT_EQ(ConfigManager::DEFAULT_PORT, config_manager.getPort());
}