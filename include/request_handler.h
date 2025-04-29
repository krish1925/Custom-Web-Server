#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include <string>

struct Request
{
    std::string raw;
    std::string uri;
    std::string method;
    std::string client_ip;
};

struct Response
{
    std::string data;
};

class IRequestHandler
{
public:
    virtual Response handle(const Request&) = 0;
    virtual ~IRequestHandler() = default;
};

#endif  // REQUEST_HANDLER_H
