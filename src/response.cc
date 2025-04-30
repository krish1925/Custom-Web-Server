#include "response.h"

std::string Response::to_string() const // class-qualified definition
{
    std::string msg = "HTTP/1.1 " + std::to_string(status_code) +
                      " " + status_message + "\r\n";
    for (const auto &h : headers)
        msg += h.first + ": " + h.second + "\r\n";
    msg += "\r\n";
    msg += body;
    return msg;
}