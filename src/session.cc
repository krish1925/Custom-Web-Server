#include "session.h"
#include <iostream>

Session::Session(boost::asio::io_service &io_service)
    : socket_(io_service), should_close_connection_(false)
{
}

Session::~Session()
{
}

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
            // Check if the request header contains "Connection: close"
            should_close_connection_ = (request_buffer_.find("Connection: close") != std::string::npos);

            // Create HTTP response with the echoed request
            std::string response = "HTTP/1.1 200 OK\r\n";
            response += "Content-Type: text/plain\r\n";

            // Echo the connection header based on client request
            if (should_close_connection_)
            {
                response += "Connection: close\r\n";
            }
            else
            {
                response += "Connection: keep-alive\r\n";
            }

            response += "Content-Length: " + std::to_string(request_buffer_.length()) + "\r\n";
            response += "\r\n";          // Empty line to separate headers from body
            response += request_buffer_; // Echo the entire request as the body

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
        delete this;
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
            delete this;
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
        delete this;
    }
}