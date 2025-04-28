#include "session.h"
#include <iostream>
#include "echo_response_handler.h"

Session::Session(boost::asio::io_service &io_service)
    : socket_(io_service),
      should_close_connection_(false),
      response_handler_(std::make_shared<EchoResponseHandler>()) {}

Session::Session(boost::asio::io_service &io_service,
                 std::shared_ptr<IResponseHandler> response_handler)
    : socket_(io_service),
      should_close_connection_(false),
      response_handler_(response_handler) {}

Session::~Session() {}

tcp::socket &Session::socket()
{
    return socket_;
}

void Session::start()
{
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
                            boost::bind(&Session::handle_read, this,
                                        boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred));
}

void Session::handle_read(const boost::system::error_code &error,
                          size_t bytes_transferred)
{
    if (!error)
    {
        // Append the new data to the request buffer
        request_buffer_.append(data_, bytes_transferred);

        // Check if the request is complete (look for "\r\n\r\n" which marks end of HTTP headers)
        if (request_buffer_.find("\r\n\r\n") != std::string::npos)
        {
            // Pass off to the response handler
            std::string response = response_handler_->generateResponse(request_buffer_, should_close_connection_);

            // Send the response
            boost::asio::async_write(socket_,
                                     boost::asio::buffer(response),
                                     boost::bind(&Session::handle_write, this,
                                                 boost::asio::placeholders::error));

            return;
        }

        // If we haven't received the complete request, read more data
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
                                boost::bind(&Session::handle_read, this,
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred));
    }
    else
    {
        destroy();
    }
}

void Session::handle_write(const boost::system::error_code &error)
{
    if (!error)
    {
        if (should_close_connection_)
        {
            // Close the connection only if requested by client
            boost::system::error_code ignored_ec;
            socket_.shutdown(tcp::socket::shutdown_both, ignored_ec);
            destroy();
            return;
        }

        // Clear the buffer for the next request
        request_buffer_.clear();

        // Start reading again for persistent connections
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
                                boost::bind(&Session::handle_read, this,
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred));
    }
    else
    {
        destroy();
    }
}
