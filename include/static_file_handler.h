#ifndef STATIC_FILE_HANDLER_H
#define STATIC_FILE_HANDLER_H

#include "request_handler.h" // needs Request / Response / interface
#include "response.h"

class StaticFileHandler : public IRequestHandler
{
public:
    StaticFileHandler() = default;
    StaticFileHandler(std::string url_prefix, std::string root_dir);

    ~StaticFileHandler() = default;

    Response handle(const Request &req) override;

private:
    std::string prefix_;
    std::string root_;
};

#endif // STATIC_FILE_HANDLER_H