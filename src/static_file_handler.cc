#include "static_file_handler.h"

StaticFileHandler::StaticFileHandler(std::string url_prefix, std::string root_dir)
    : prefix_(std::move(url_prefix)),
      root_(std::move(root_dir)) {}

Response StaticFileHandler::handle(const Request &req)
{
    Response res;
    res.data = std::string("STATIC_FILE_HANDLER (prefix=") + prefix_ +
               ", root=" + root_ + "): " + req.raw;
    return res;
}