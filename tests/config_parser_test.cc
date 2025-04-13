#include "gtest/gtest.h"
#include "config_parser.h"

class NginxConfigParserTest : public testing::Test
{
protected:
    NginxConfigParser parser;
    NginxConfig out_config;
};

TEST(NginxConfigParserTest, SimpleConfig)
{
    bool success = parser.Parse("example_config", &out_config);
    EXPECT_TRUE(success);
}

TEST(NginxConfigParserTest, ParseMoreConfig)
{
    bool success = parser.Parse("more_config", &out_config);
    EXPECT_TRUE(success);
}

TEST(NginxConfigParserTest, ParseEmptyConfig)
{
    bool success = parser.Parse("empty_config", &out_config);
    EXPECT_FALSE(success);
}

TEST(NginxConfigParserTest, MissingBracesConfig)
{
    bool success = parser.Parse("unbalanced_config", &out_config);
    EXPECT_FALSE(success);
}

TEST(NginxConfigParserTest, LevelsConfig)
{
    bool success = parser.Parse("levels_config", &out_config);
    EXPECT_TRUE(success);
}

TEST(NginxConfigParserTest, BracketConfig)
{
    bool success = parser.Parse("bracket_config", &out_config);
    EXPECT_FALSE(success);
}

TEST(NginxConfigParserTest, MissingSemicolonConfig)
{
    bool success = parser.Parse("missing_semicolon_config", &out_config);
    EXPECT_FALSE(success);
}

TEST(NginxConfigParserTest, CommentConfig)
{
    bool success = parser.Parse("comment_config", &out_config);
    EXPECT_FALSE(success);
}