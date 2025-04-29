#include "echo_handler.h"
#include <string>

Response EchoHandler::handle(const Request& req)
{
    Response res;
    res.data = "ECHO_HANDLER: HTTP/1.1 200 OK\r\n"
               "Content-Type: text/plain\r\n"
               "Content-Length: " + std::to_string(req.raw.size()) +
               "\r\n\r\n" +
               req.raw;
    return res;
}
