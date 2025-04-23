#include "gtest/gtest.h"
#include "config_parser.h"

class NginxConfigParserTest : public testing::Test
{
protected:
    NginxConfigParser parser;
    NginxConfig out_config;
};

// Added missing test fixture class
class NginxConfigToStringTest : public testing::Test
{
protected:
    NginxConfigParser parser;
};

TEST_F(NginxConfigParserTest, SimpleConfig)
{
    // Use in-memory config instead of file
    std::string config_str = "server {\n  listen 80;\n  server_name example.com;\n}\n";
    std::stringstream config_stream(config_str);
    EXPECT_TRUE(parser.Parse(&config_stream, &out_config));
}

TEST_F(NginxConfigParserTest, ParseMoreConfig)
{
    // Use in-memory config instead of file
    std::string config_str =
        "http {\n"
        "  server {\n"
        "    listen 8080;\n"
        "    server_name example.org;\n"
        "  }\n"
        "}\n";
    std::stringstream config_stream(config_str);
    EXPECT_TRUE(parser.Parse(&config_stream, &out_config));
}

TEST_F(NginxConfigParserTest, ParseEmptyConfig)
{
    // Still expect false for non-existent file
    bool success = parser.Parse("non_existent_file", &out_config);
    EXPECT_FALSE(success);
}

TEST_F(NginxConfigParserTest, MissingBracesConfig)
{
    // Use in-memory config with unbalanced braces
    std::string config_str = "server {\n  listen 80;\n"; // Missing closing brace
    std::stringstream config_stream(config_str);
    EXPECT_FALSE(parser.Parse(&config_stream, &out_config));
}

TEST_F(NginxConfigParserTest, LevelsConfig)
{
    // Use in-memory config with nested levels
    std::string config_str =
        "http {\n"
        "  server {\n"
        "    location / {\n"
        "      proxy_pass http://localhost:8080;\n"
        "    }\n"
        "  }\n"
        "}\n";
    std::stringstream config_stream(config_str);
    EXPECT_TRUE(parser.Parse(&config_stream, &out_config));
}

TEST_F(NginxConfigParserTest, BracketConfig)
{
    // Use in-memory config with invalid brackets
    std::string config_str =
        "server {\n"
        "  listen 80;\n"
        "  location / [\n"
        "    root /var/www/html;\n"
        "  ]\n"
        "}\n";
    std::stringstream config_stream(config_str);
    EXPECT_FALSE(parser.Parse(&config_stream, &out_config));
}

TEST_F(NginxConfigParserTest, CommentConfig)
{
    // Use in-memory config with comment
    std::string config_str =
        "# This is a comment\n"
        "server # Another comment that breaks the config\n";
    std::stringstream config_stream(config_str);
    EXPECT_FALSE(parser.Parse(&config_stream, &out_config));
}

TEST_F(NginxConfigParserTest, TokenTypeAsString_AllValues)
{
    for (int t = NginxConfigParser::TOKEN_TYPE_START;
         t <= NginxConfigParser::TOKEN_TYPE_ERROR; ++t)
    {
        const char *s = parser.TokenTypeAsString(
            static_cast<NginxConfigParser::TokenType>(t));
        EXPECT_NE(std::string(s), "Unknown token type");
    }
}

TEST_F(NginxConfigParserTest, ParseToken_QuotedAndError)
{
    std::istringstream in("\"hello world\" 'foo bar' \"unterminated");
    std::string val;

    // Double‑quote complete
    EXPECT_EQ(parser.ParseToken(&in, &val), NginxConfigParser::TOKEN_TYPE_NORMAL);
    EXPECT_EQ(val, "\"hello world\"");

    // Single‑quote complete
    val.clear();
    EXPECT_EQ(parser.ParseToken(&in, &val), NginxConfigParser::TOKEN_TYPE_NORMAL);
    EXPECT_EQ(val, "'foo bar'");

    // Unterminated double‑quote → error
    val.clear();
    EXPECT_EQ(parser.ParseToken(&in, &val), NginxConfigParser::TOKEN_TYPE_ERROR);
}

TEST_F(NginxConfigParserTest, Serialization_RoundTrip)
{
    constexpr char text[] =
        "http {\n"
        "  server {\n"
        "    listen 8080;\n"
        "  }\n"
        "}\n";
    NginxConfig cfg;
    std::istringstream in(text);
    EXPECT_TRUE(parser.Parse(&in, &cfg));

    const auto out = cfg.ToString();
    EXPECT_EQ(out, text);
}

TEST_F(NginxConfigParserTest, Parse_FileNotFound)
{
    NginxConfig cfg;
    EXPECT_FALSE(parser.Parse("this_file_does_not_exist.conf", &cfg));
}

TEST_F(NginxConfigParserTest, SimpleConfigFromStringStream)
{
    std::stringstream ss("foo bar;");
    EXPECT_TRUE(parser.Parse(&ss, &out_config));
    EXPECT_EQ(1, out_config.statements_.size());
    EXPECT_EQ("foo", out_config.statements_[0]->tokens_[0]);
    EXPECT_EQ("bar", out_config.statements_[0]->tokens_[1]);
}

TEST_F(NginxConfigParserTest, SimpleConfigWithComment)
{
    std::stringstream ss("#comment\nfoo bar; # another comment");
    EXPECT_TRUE(parser.Parse(&ss, &out_config));
    EXPECT_EQ(1, out_config.statements_.size());
    EXPECT_EQ("foo", out_config.statements_[0]->tokens_[0]);
    EXPECT_EQ("bar", out_config.statements_[0]->tokens_[1]);
}

TEST_F(NginxConfigParserTest, NestedBlockConfig)
{
    std::stringstream ss("server { listen 80; }");
    EXPECT_TRUE(parser.Parse(&ss, &out_config));
    EXPECT_EQ(1, out_config.statements_.size());
    ASSERT_NE(nullptr, out_config.statements_[0]->child_block_.get());
    EXPECT_EQ(1, out_config.statements_[0]->child_block_->statements_.size());
    EXPECT_EQ("listen", out_config.statements_[0]->child_block_->statements_[0]->tokens_[0]);
    EXPECT_EQ("80", out_config.statements_[0]->child_block_->statements_[0]->tokens_[1]);
}

TEST_F(NginxConfigParserTest, MultipleStatements)
{
    std::stringstream ss("foo bar; server { listen 80; } baz qux;");
    EXPECT_TRUE(parser.Parse(&ss, &out_config));
    EXPECT_EQ(3, out_config.statements_.size());
    EXPECT_EQ("foo", out_config.statements_[0]->tokens_[0]);
    EXPECT_EQ("server", out_config.statements_[1]->tokens_[0]);
    EXPECT_EQ("baz", out_config.statements_[2]->tokens_[0]);
}

TEST_F(NginxConfigParserTest, UnbalancedBracesOpen)
{
    std::stringstream ss("server { listen 80;"); // Missing closing brace
    EXPECT_FALSE(parser.Parse(&ss, &out_config));
}

TEST_F(NginxConfigParserTest, UnbalancedBracesClose)
{
    std::stringstream ss("server listen 80; }"); // Extra closing brace
    EXPECT_FALSE(parser.Parse(&ss, &out_config));
}

TEST_F(NginxConfigParserTest, InvalidStatementMissingSemicolon)
{
    std::stringstream ss("foo bar"); // Missing semicolon
    EXPECT_FALSE(parser.Parse(&ss, &out_config));
}

TEST_F(NginxConfigParserTest, InvalidStatementStartWithSemicolon)
{
    std::stringstream ss("; foo bar;");
    EXPECT_FALSE(parser.Parse(&ss, &out_config));
}

TEST_F(NginxConfigParserTest, BlockNotFollowingToken)
{
    std::stringstream ss("{ listen 80; }"); // Block without statement name
    EXPECT_FALSE(parser.Parse(&ss, &out_config));
}

TEST_F(NginxConfigParserTest, StatementInsideBlockMissingSemicolon)
{
    std::stringstream ss("server { listen 80 }"); // Missing semicolon inside block
    EXPECT_FALSE(parser.Parse(&ss, &out_config));
}

TEST_F(NginxConfigParserTest, UnexpectedToken)
{
    std::stringstream ss("foo bar { ; }"); // Semicolon right after brace
    EXPECT_FALSE(parser.Parse(&ss, &out_config));
}

TEST_F(NginxConfigToStringTest, BasicToString)
{
    NginxConfigStatement statement;
    statement.tokens_.push_back("foo");
    statement.tokens_.push_back("bar");
    std::string expected = "foo bar;\n";
    EXPECT_EQ(expected, statement.ToString(0));
}

TEST_F(NginxConfigToStringTest, NestedToString)
{
    NginxConfig config;
    auto stmt1 = std::make_shared<NginxConfigStatement>();
    stmt1->tokens_.push_back("server");
    stmt1->child_block_ = std::unique_ptr<NginxConfig>(new NginxConfig());
    auto stmt2 = std::make_shared<NginxConfigStatement>();
    stmt2->tokens_.push_back("listen");
    stmt2->tokens_.push_back("80");
    stmt1->child_block_->statements_.push_back(stmt2);
    config.statements_.push_back(stmt1);

    std::string expected = "server {\n  listen 80;\n}\n";
    EXPECT_EQ(expected, config.ToString(0));
}

// Test ParseToken edge cases
TEST_F(NginxConfigParserTest, ParseTokenEdgeCases)
{
    std::string value;

    // Test empty quoted strings
    std::stringstream ss1("''");
    EXPECT_EQ(parser.ParseToken(&ss1, &value), NginxConfigParser::TOKEN_TYPE_NORMAL);
    EXPECT_EQ("''", value);

    // Test double quoted empty string
    value.clear();
    std::stringstream ss2("\"\"");
    EXPECT_EQ(parser.ParseToken(&ss2, &value), NginxConfigParser::TOKEN_TYPE_NORMAL);
    EXPECT_EQ("\"\"", value);

    // Test unterminated single quote
    value.clear();
    std::stringstream ss3("'abc");
    EXPECT_EQ(parser.ParseToken(&ss3, &value), NginxConfigParser::TOKEN_TYPE_ERROR);
}

TEST_F(NginxConfigParserTest, ComplexToString)
{
    // Test complex nested config string representation
    NginxConfig config;

    // Create a complex nested structure
    auto stmt1 = std::make_shared<NginxConfigStatement>();
    stmt1->tokens_.push_back("http");
    stmt1->child_block_ = std::unique_ptr<NginxConfig>(new NginxConfig());

    auto stmt2 = std::make_shared<NginxConfigStatement>();
    stmt2->tokens_.push_back("server");
    stmt2->child_block_ = std::unique_ptr<NginxConfig>(new NginxConfig());

    auto stmt3 = std::make_shared<NginxConfigStatement>();
    stmt3->tokens_.push_back("listen");
    stmt3->tokens_.push_back("8080");

    auto stmt4 = std::make_shared<NginxConfigStatement>();
    stmt4->tokens_.push_back("server_name");
    stmt4->tokens_.push_back("localhost");

    stmt2->child_block_->statements_.push_back(stmt3);
    stmt2->child_block_->statements_.push_back(stmt4);
    stmt1->child_block_->statements_.push_back(stmt2);
    config.statements_.push_back(stmt1);

    std::string expected = "http {\n  server {\n    listen 8080;\n    server_name localhost;\n  }\n}\n";
    EXPECT_EQ(expected, config.ToString(0));
}

TEST_F(NginxConfigParserTest, ParseErrorCases)
{
    // Test various error conditions in parsing
    std::stringstream ss1("foo bar { baz qux"); // Unterminated block
    EXPECT_FALSE(parser.Parse(&ss1, &out_config));

    std::stringstream ss2("foo bar { baz qux; } }"); // Extra closing brace
    EXPECT_FALSE(parser.Parse(&ss2, &out_config));

    std::stringstream ss3("foo bar { baz qux; } {"); // Extra opening brace
    EXPECT_FALSE(parser.Parse(&ss3, &out_config));

    std::stringstream ss4("foo bar { baz qux; } server { listen 80;"); // Nested unterminated block
    EXPECT_FALSE(parser.Parse(&ss4, &out_config));
}

TEST_F(NginxConfigParserTest, ParseComplexConfig)
{
    // Test parsing a complex configuration with multiple nested blocks
    std::stringstream ss(
        "http {\n"
        "  server {\n"
        "    listen 8080;\n"
        "    server_name localhost;\n"
        "    location / {\n"
        "      root /var/www/html;\n"
        "      index index.html;\n"
        "    }\n"
        "  }\n"
        "}\n");

    EXPECT_TRUE(parser.Parse(&ss, &out_config));
    EXPECT_EQ(1, out_config.statements_.size());
    EXPECT_EQ("http", out_config.statements_[0]->tokens_[0]);

    auto &server_block = out_config.statements_[0]->child_block_->statements_[0];
    EXPECT_EQ("server", server_block->tokens_[0]);
    EXPECT_EQ(3, server_block->child_block_->statements_.size());

    auto &location_block = server_block->child_block_->statements_[2];
    EXPECT_EQ("location", location_block->tokens_[0]);
    EXPECT_EQ("/", location_block->tokens_[1]);
    EXPECT_EQ(2, location_block->child_block_->statements_.size());
}

// Test ParseTokenSpecialCases
TEST_F(NginxConfigParserTest, ParseTokenSpecialCases)
{
    std::string value;

    // Test empty input
    std::stringstream ss1("");
    EXPECT_EQ(parser.ParseToken(&ss1, &value), NginxConfigParser::TOKEN_TYPE_EOF);

    // Test just whitespace
    value.clear();
    std::stringstream ss2("  \t\n\r");
    EXPECT_EQ(parser.ParseToken(&ss2, &value), NginxConfigParser::TOKEN_TYPE_EOF);

    // Test simple comment
    value.clear();
    std::stringstream ss3("# comment\n");
    EXPECT_EQ(parser.ParseToken(&ss3, &value), NginxConfigParser::TOKEN_TYPE_COMMENT);
}

TEST_F(NginxConfigParserTest, TokenTypeAsString_AllAndUnknown)
{
    // all valid
    for (int t = NginxConfigParser::TOKEN_TYPE_START;
         t <= NginxConfigParser::TOKEN_TYPE_ERROR; ++t)
    {
        auto s = parser.TokenTypeAsString(static_cast<NginxConfigParser::TokenType>(t));
        EXPECT_NE(std::string(s), "Unknown token type");
    }
    // and one definitely invalid
    EXPECT_STREQ(
        "Unknown token type",
        parser.TokenTypeAsString(static_cast<NginxConfigParser::TokenType>(999)));
}

TEST_F(NginxConfigParserTest, ConfigFile)
{
    bool success = parser.Parse("example_config", &out_config);
    EXPECT_TRUE(success);
}