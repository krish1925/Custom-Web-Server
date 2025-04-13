#include "gtest/gtest.h"
#include "config_parser.h"

TEST(NginxConfigParserTest, SimpleConfig)
{
    NginxConfigParser parser;
    NginxConfig out_config;

    bool success = parser.Parse("example_config", &out_config);

    EXPECT_TRUE(success);
}

TEST(NginxConfigParserTest, ParseMoreConfig)
{
    NginxConfigParser parser;
    NginxConfig out_config;

    bool success = parser.Parse("more_config", &out_config);
    EXPECT_TRUE(success);
}

TEST(NginxConfigParserTest, ParseEmptyConfig)
{
    NginxConfigParser parser;
    NginxConfig out_config;

    bool success = parser.Parse("empty_config", &out_config);
    EXPECT_FALSE(success);
}

TEST(NginxConfigParserTest, MissingBracesConfig)
{
    NginxConfigParser parser;
    NginxConfig out_config;

    bool success = parser.Parse("unbalanced_config", &out_config);
    EXPECT_FALSE(success);
}

TEST(NginxConfigParserTest, LevelsConfig)
{
    NginxConfigParser parser;
    NginxConfig out_config;

    bool success = parser.Parse("levels_config", &out_config);
    EXPECT_TRUE(success);
}

TEST(NginxConfigParserTest, BracketConfig)
{
    NginxConfigParser parser;
    NginxConfig out_config;

    bool success = parser.Parse("bracket_config", &out_config);
    EXPECT_FALSE(success);
}

TEST(NginxConfigParserTest, MissingSemicolonConfig)
{
    NginxConfigParser parser;
    NginxConfig out_config;

    bool success = parser.Parse("missing_semicolon_config", &out_config);
    EXPECT_FALSE(success);
}

TEST(NginxConfigParserTest, CommentConfig)
{
    NginxConfigParser parser;
    NginxConfig out_config;

    bool success = parser.Parse("comment_config", &out_config);
    EXPECT_FALSE(success);
}