#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include "response.h"
#include <string>

struct Request
{
    std::string raw;
    std::string uri;
    std::string method;
    std::string client_ip;
};

class IRequestHandler
{
public:
    virtual Response handle(const Request &req, bool &should_close) = 0;
    virtual ~IRequestHandler() = default;
};

#endif // REQUEST_HANDLER_H
