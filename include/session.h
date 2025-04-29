#ifndef SESSION_H
#define SESSION_H
#include "config_manager.h"
#include "request_handler.h"
#include "echo_handler.h"
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <string>
#include <memory>

using boost::asio::ip::tcp;

class IResponseHandler {
public:
    virtual ~IResponseHandler() {}
    
    // generate HTTP response from request
    virtual std::string generateResponse(const std::string& request, bool& should_close) = 0;
};

// default implementation of response handler
class EchoResponseHandler : public IResponseHandler {
public:
    // generate a simple echo response
    std::string generateResponse(const std::string& request, bool& should_close) override;
};

class Session
{
public:
    Session(boost::asio::io_service& io,
            const ConfigManager& config);

    tcp::socket& socket() { return socket_; }
    void start();

private:
    void handle_read(const boost::system::error_code&, size_t);
    void handle_write(const boost::system::error_code&);

    tcp::socket socket_;
    enum { max_length = 8192 };
    char data_[max_length];
    std::string buffer_;  
    bool should_close_{false};

    const ConfigManager& cfg_; 
    std::shared_ptr<IRequestHandler> default_echo_ =
             std::make_shared<EchoHandler>(); 
};
#endif