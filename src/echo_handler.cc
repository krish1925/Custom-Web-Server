#include "echo_handler.h"
#include "response.h"
#include "logging.h"

Response EchoHandler::handle(const Request& req)
{
    BOOST_LOG_TRIVIAL(info)  << "[EchoHandler] Handling request";
    BOOST_LOG_TRIVIAL(debug) << "[EchoHandler] Request URI: " << req.uri
                             << ", method: " << req.method
                             << ", client_ip: " << req.client_ip
                             << ", size: " << req.raw.size();

    Response res;

    // 1. Status line
    res.status_code    = 200;
    res.status_message = "OK";

    // 2. Body (echo the raw request with a prefix)
    res.body = "ECHO_HANDLER: " + req.raw;

    // 3. Required headers
    res.headers["Content-Type"]   = "text/plain";
    res.headers["Content-Length"] = std::to_string(res.body.size());

    return res;
}
