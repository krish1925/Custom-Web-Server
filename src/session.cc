#include "session.h"
#include "echo_handler.h"
#include <boost/bind.hpp>
#include <sstream>

Session::Session(boost::asio::io_service &io, const ConfigManager &config)
    : socket_(io), cfg_(config),
      default_echo_(std::make_shared<EchoHandler>()) {}

Session::Session(boost::asio::io_service &io_service, const ConfigManager &config,
                 std::shared_ptr<IResponseHandler> response_handler)
    : socket_(io_service), cfg_(config), default_echo_(std::make_shared<EchoHandler>()),
      response_handler_(response_handler) {}

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

void Session::handle_read(const boost::system::error_code &ec,
                          size_t n)
{
    if (ec)
    {
        return;
    }

    buffer_.append(data_, n);

    std::cout << "[Session] Received " << n << " bytes\n";
    std::cout << "[Session] Chunk: \""
              << std::string(data_, data_ + n)
              << "\"\n";
    std::cout << "[Session] Buffer: \"" << buffer_ << "\"\n";

    if (buffer_.find("\r\n\r\n") == std::string::npos)
    {
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
                                boost::bind(&Session::handle_read, this,
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred));
        std::cout << "[Session] Waiting for more data...\n";
        return;
    }

    std::istringstream iss(buffer_);
    std::string method, uri, version;
    iss >> method >> uri >> version;
    std::cout << "[Session] Method: " << method << "\n";
    std::cout << "[Session] URI: " << uri << "\n";
    std::cout << "[Session] Version: " << version << "\n";

    should_close_ = (buffer_.find("Connection: close") != std::string::npos);

    Request req;
    req.raw = buffer_;
    req.uri = uri;
    req.method = method;
    try
    {
        req.client_ip = socket_.remote_endpoint().address().to_string();
    }
    catch (...)
    {
        req.client_ip = "unknown";
    }

    auto handler_opt = cfg_.matchHandler(uri);
    auto handler = handler_opt ? *handler_opt : default_echo_;

    Response resp = handler->handle(req);

    boost::asio::async_write(socket_, boost::asio::buffer(resp.data),
                             boost::bind(&Session::handle_write, this,
                                         boost::asio::placeholders::error));
}

void Session::handle_write(const boost::system::error_code &ec)
{
    if (ec)
        return;

    if (should_close_)
    {
        boost::system::error_code ig;
        socket_.shutdown(tcp::socket::shutdown_both, ig);
        return;
    }

    buffer_.clear();
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
                            boost::bind(&Session::handle_read, this,
                                        boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred));
}
std::string EchoResponseHandler::generateResponse(const std::string &request, bool &should_close)
{
    // Check if the request header contains "Connection: close"
    should_close = (request.find("Connection: close") != std::string::npos);

    // Create HTTP response with the echoed request
    std::string response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: text/plain\r\n";

    // Echo the connection header based on client request
    if (should_close)
    {
        response += "Connection: close\r\n";
    }
    else
    {
        response += "Connection: keep-alive\r\n";
    }

    response += "Content-Length: " + std::to_string(request.length()) + "\r\n";
    response += "\r\n";  // Empty line to separate headers from body
    response += request; // Echo the entire request as the body

    return response;
}
