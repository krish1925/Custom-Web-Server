#include "gtest/gtest.h"
#include "static_file_handler.h"
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

class StaticFileHandlerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create test directory structure
        fs::create_directories("test_root/subdir");

        // Create test files with content
        std::ofstream text_file("test_root/test.txt");
        text_file << "This is a test text file";
        text_file.close();

        std::ofstream html_file("test_root/test.html");
        html_file << "<html><body>Test HTML</body></html>";
        html_file.close();

        std::ofstream js_file("test_root/script.js");
        js_file << "function test() { return 'hello'; }";
        js_file.close();

        std::ofstream subdir_file("test_root/subdir/nested.txt");
        subdir_file << "Nested file content";
        subdir_file.close();
    }

    void TearDown() override
    {
        // Clean up test files after each test
        fs::remove_all("test_root");
    }
};

// Test successful file serving
TEST_F(StaticFileHandlerTest, ServesExistingFile)
{
    StaticFileHandler handler("/static", "test_root");
    Request req;
    req.uri = "/static/test.txt";

    Response res = handler.handle(req);

    EXPECT_EQ(200, res.status_code);
    EXPECT_EQ("OK", res.status_message);
    EXPECT_EQ("text/plain", res.headers["Content-Type"]);
    EXPECT_EQ("This is a test text file", res.body);
    EXPECT_EQ(std::to_string(res.body.size()), res.headers["Content-Length"]);
}

// Test correct MIME type detection
TEST_F(StaticFileHandlerTest, CorrectMimeTypes)
{
    StaticFileHandler handler("/static", "test_root");

    // HTML file
    Request html_req;
    html_req.uri = "/static/test.html";
    Response html_res = handler.handle(html_req);
    EXPECT_EQ("text/html", html_res.headers["Content-Type"]);

    // JavaScript file
    Request js_req;
    js_req.uri = "/static/script.js";
    Response js_res = handler.handle(js_req);
    EXPECT_EQ("application/javascript", js_res.headers["Content-Type"]);
}

// Test nested directory access
TEST_F(StaticFileHandlerTest, NestedDirectoryAccess)
{
    StaticFileHandler handler("/static", "test_root");
    Request req;
    req.uri = "/static/subdir/nested.txt";

    Response res = handler.handle(req);

    EXPECT_EQ(200, res.status_code);
    EXPECT_EQ("Nested file content", res.body);
}

// Test URL doesn't match prefix
TEST_F(StaticFileHandlerTest, UrlDoesntMatchPrefix)
{
    StaticFileHandler handler("/static", "test_root");
    Request req;
    req.uri = "/files/test.txt"; // Wrong prefix

    Response res = handler.handle(req);

    EXPECT_EQ(400, res.status_code);
    EXPECT_EQ("Bad Request", res.status_message);
    EXPECT_TRUE(res.body.find("URL does not match") != std::string::npos);
}

// Test non-existent file
TEST_F(StaticFileHandlerTest, NonExistentFile)
{
    StaticFileHandler handler("/static", "test_root");
    Request req;
    req.uri = "/static/doesnotexist.txt";

    Response res = handler.handle(req);

    EXPECT_EQ(404, res.status_code);
    EXPECT_EQ("Not Found", res.status_message);
}

// Test directory access (should 404)
TEST_F(StaticFileHandlerTest, DirectoryAccess)
{
    StaticFileHandler handler("/static", "test_root");
    Request req;
    req.uri = "/static/subdir";

    Response res = handler.handle(req);

    EXPECT_EQ(404, res.status_code);
    EXPECT_EQ("Not Found", res.status_message);
}

// Test path traversal prevention
TEST_F(StaticFileHandlerTest, PathTraversalPrevention)
{
    // Create a file outside the root directory
    std::ofstream outside_file("outside.txt");
    outside_file << "This file is outside the root";
    outside_file.close();

    StaticFileHandler handler("/static", "test_root");
    Request req;
    req.uri = "/static/../outside.txt"; // Try to access outside the root

    Response res = handler.handle(req);

    EXPECT_EQ(404, res.status_code);

    // Clean up
    fs::remove("outside.txt");
}

// Test prefix normalization
TEST_F(StaticFileHandlerTest, PrefixNormalization)
{
    // Handler without trailing slash in prefix
    StaticFileHandler handler("/api", "test_root");
    Request req;
    req.uri = "/api/test.txt";

    Response res = handler.handle(req);

    EXPECT_EQ(200, res.status_code);
    EXPECT_EQ("This is a test text file", res.body);
}

// Test empty path
TEST_F(StaticFileHandlerTest, EmptyPath)
{
    StaticFileHandler handler("/static/", "test_root");
    Request req;
    req.uri = "/static/"; // No specific file

    Response res = handler.handle(req);

    EXPECT_EQ(404, res.status_code);
}