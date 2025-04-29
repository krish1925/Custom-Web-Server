#include "echo_response_handler.h"

std::string EchoResponseHandler::generateResponse(const std::string &request, bool &should_close)
{
    // Check if the request header contains "Connection: close"
    should_close = (request.find("Connection: close") != std::string::npos);

    // Create HTTP response with the echoed request
    std::string response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: text/plain\r\n";

    // Echo the connection header based on client request
    if (should_close)
    {
        response += "Connection: close\r\n";
    }
    else
    {
        response += "Connection: keep-alive\r\n";
    }

    response += "Content-Length: " + std::to_string(request.length()) + "\r\n";
    response += "\r\n";  // Empty line to separate headers from body
    response += request; // Echo the entire request as the body

    return response;
}