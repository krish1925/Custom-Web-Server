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
class Session {
public:
    // Constructor with io_service for production
    Session(boost::asio::io_service &io_service);

    // Constructor for testing with injected response handler
    Session(boost::asio::io_service &io_service, 
            std::shared_ptr<IResponseHandler> response_handler);

    virtual ~Session();

    // Get the socket reference
    virtual tcp::socket &socket();

    // Start the session
    virtual void start();

protected:
    // Read handler
    virtual void handle_read(const boost::system::error_code &error,
                             size_t bytes_transferred);

    // Write handler
    virtual void handle_write(const boost::system::error_code &error);

    // Virtual method for clean object deletion (overridable for test safety)
    virtual void destroy() {
        delete this;
    }

    tcp::socket socket_;
    enum {
        max_length = 8192
    };
    char data_[max_length];
    std::string request_buffer_;
    bool should_close_connection_;
    std::shared_ptr<IResponseHandler> response_handler_;
};

// Interface for response generation
class IResponseHandler {
public:
    virtual ~IResponseHandler() {}

    // Generate response and decide connection persistence
    virtual std::string generateResponse(const std::string& request, bool& should_close) = 0;
};

// Default echo implementation
class EchoResponseHandler : public IResponseHandler {
public:
    std::string generateResponse(const std::string& request, bool& should_close) override;
};

#endif // SESSION_H
