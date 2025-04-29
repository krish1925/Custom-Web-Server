#ifndef RESPONSE_H
#define RESPONSE_H

#include <string> 
#include <unordered_map> 

struct Response {
    int status_code            = 200;
    std::string status_message = "OK";
    std::unordered_map<std::string,std::string> headers;
    std::string body;

    std::string to_string() const;      // <— new member
};

#endif // RESPONSE_H