#include "echo_handler.h"
#include "response.h"
#include "logging.h"

Response EchoHandler::handle(const Request &req, bool &should_close)
{
    BOOST_LOG_TRIVIAL(info) << "[EchoHandler] Handling request";
    BOOST_LOG_TRIVIAL(debug) << "[EchoHandler] Request URI: " << req.uri
                             << ", method: " << req.method
                             << ", client_ip: " << req.client_ip
                             << ", size: " << req.raw.size();

    should_close = (req.raw.find("Connection: close") != std::string::npos);
    BOOST_LOG_TRIVIAL(info) << "[Session] Connection-close flag: "
                            << (should_close ? "true" : "false");
    Response res;

    // 1. Status line
    res.status_code = 200;
    res.status_message = "OK";

    // 2. Body (echo the raw request)
    res.body = req.raw;

    // 3. Required headers
    res.headers["Content-Type"] = "text/plain";
    res.headers["Connection"] = should_close ? "close" : "keep-alive";
    res.headers["Content-Length"] = std::to_string(res.body.size());

    return res;
}
