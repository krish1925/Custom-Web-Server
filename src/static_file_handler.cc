#include "static_file_handler.h"
#include <string>

Response StaticFileHandler::handle(const Request& req)
{
    Response res;
    res.data = std::string("STATIC_FILE_HANDLER (prefix=") + prefix_ +
               ", root=" + root_ + "): " + req.raw;
    return res;
}