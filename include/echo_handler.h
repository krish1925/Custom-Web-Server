#ifndef ECHO_HANDLER_H
#define ECHO_HANDLER_H

#include "request_handler.h"   // needs Request / Response / interface

class EchoHandler : public IRequestHandler
{
public:
    EchoHandler()  = default;
    ~EchoHandler() = default;

    Response handle(const Request& req) override;
};

#endif  // ECHO_HANDLER_H
