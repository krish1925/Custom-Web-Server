#ifndef RESPONSE_HANDLER_H
#define RESPONSE_HANDLER_H

#include <string>

class IResponseHandler
{
public:
    virtual ~IResponseHandler() {}

    // generate HTTP response from request
    virtual std::string generateResponse(const std::string &request, bool &should_close) = 0;
};

#endif // RESPONSE_HANDLER_H