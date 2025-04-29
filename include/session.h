#ifndef SESSION_H
#define SESSION_H

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <string>
#include <memory>

using boost::asio::ip::tcp;

// Forward declaration for the response handler interface
class IResponseHandler;

// Session class that handles client connections
class Session
{
public:
    // Constructor with dependency injection for io_service
    Session(boost::asio::io_service &io_service);
    
    // Constructor with dependency injection for testing
    Session(boost::asio::io_service &io_service, 
            std::shared_ptr<IResponseHandler> response_handler);
    
    virtual ~Session();

    // Get socket reference
    virtual tcp::socket &socket();
    
    // Start the session
    virtual void start();

protected:
    // made protected for testing
    virtual void handle_read(const boost::system::error_code &error,
                     size_t bytes_transferred);
    virtual void handle_write(const boost::system::error_code &error);
    
    // Virtual method for clean object deletion (overridable for test safety)
    virtual void destroy() {
        delete this;
    }

    tcp::socket socket_;
    enum
    {
        max_length = 8192
    };
    char data_[max_length];
    std::string request_buffer_;
    bool should_close_connection_;
    std::shared_ptr<IResponseHandler> response_handler_;
};

// interface for response handling
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

#endif // SESSION_H
