#ifndef ECHO_RESPONSE_HANDLER_H
#define ECHO_RESPONSE_HANDLER_H

#include "response_handler.h"

class EchoResponseHandler : public IResponseHandler
{
public:
    virtual ~EchoResponseHandler() override;
    // generate a simple echo response
    std::string generateResponse(const std::string &request, bool &should_close) override;
};

#endif // ECHO_RESPONSE_HANDLER_H