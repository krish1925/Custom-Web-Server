#include "session.h"
#include "echo_handler.h"
#include "logging.h"
#include <boost/bind.hpp>
#include <sstream>

Session::Session(boost::asio::io_service &io, const ConfigManager &config)
    : socket_(io), cfg_(config),
      default_echo_(std::make_shared<EchoHandler>())
{
    BOOST_LOG_TRIVIAL(info) << "[Session] Constructed (default handler)";
}

Session::Session(boost::asio::io_service &io_service, const ConfigManager &config,
                 std::shared_ptr<IRequestHandler> request_handler)
    : socket_(io_service), cfg_(config), default_echo_(std::make_shared<EchoHandler>())
{
    BOOST_LOG_TRIVIAL(info) << "[Session] Constructed (custom handler)";
}

tcp::socket &Session::socket()
{
    return socket_;
}

void Session::start()
{
    BOOST_LOG_TRIVIAL(info) << "[Session] Starting async read";
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
        BOOST_LOG_TRIVIAL(error) << "[Session] Read error: " << ec.message();
        return;
    }

    buffer_.append(data_, n);

    BOOST_LOG_TRIVIAL(debug) << "[Session] Received " << n << " bytes";
    BOOST_LOG_TRIVIAL(debug) << "[Session] Chunk: \""
                             << std::string(data_, data_ + n)
                             << "\"";
    BOOST_LOG_TRIVIAL(debug) << "[Session] Buffer now: \""
                             << buffer_
                             << "\"";

    if (buffer_.find("\r\n\r\n") == std::string::npos)
    {
        BOOST_LOG_TRIVIAL(debug) << "[Session] Incomplete header, waiting for more data";
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
                                boost::bind(&Session::handle_read, this,
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred));
        return;
    }

    BOOST_LOG_TRIVIAL(info) << "[Session] Full HTTP header received";
    std::istringstream iss(buffer_);
    std::string method, uri, version;
    iss >> method >> uri >> version;
    BOOST_LOG_TRIVIAL(info) << "[Session] Method: " << method;
    BOOST_LOG_TRIVIAL(info) << "[Session] URI:    " << uri;
    BOOST_LOG_TRIVIAL(info) << "[Session] Version:" << version;

    Request req;
    req.raw = buffer_;
    req.uri = uri;
    req.method = method;
    try
    {
        req.client_ip = socket_.remote_endpoint().address().to_string();
        BOOST_LOG_TRIVIAL(info) << "[Session] Client IP: " << req.client_ip;
    }
    catch (const std::exception &e)
    {
        req.client_ip = "unknown";
        BOOST_LOG_TRIVIAL(warning) << "[Session] Failed to retrieve client IP: "
                                   << e.what();
    }

    auto handler_opt = cfg_.matchHandler(uri);
    if (handler_opt)
    {
        BOOST_LOG_TRIVIAL(info) << "[Session] Matched custom IRequestHandler";
    }
    else
    {
        BOOST_LOG_TRIVIAL(info) << "[Session] No match; using default EchoHandler";
    }
    auto handler = handler_opt ? *handler_opt : default_echo_;

    Response resp = handler->handle(req, should_close_);
    BOOST_LOG_TRIVIAL(debug) << "[Session] Generated response of size "
                             << resp.body.size() << " bytes";

    auto outbound = std::make_shared<std::string>(resp.to_string());

    boost::asio::async_write(
        socket_, boost::asio::buffer(*outbound),
        [this, outbound](const boost::system::error_code& ec, std::size_t)
        {
            handle_write(ec);
        });
}

void Session::handle_write(const boost::system::error_code &ec)
{
    if (ec)
    {
        BOOST_LOG_TRIVIAL(error) << "[Session] Write error: " << ec.message();
        return;
    }

    BOOST_LOG_TRIVIAL(debug) << "[Session] Write complete";

    if (should_close_)
    {
        BOOST_LOG_TRIVIAL(info) << "[Session] Shutting down socket per client request";
        boost::system::error_code ig;
        socket_.shutdown(tcp::socket::shutdown_both, ig);
        return;
    }

    buffer_.clear();
    BOOST_LOG_TRIVIAL(debug) << "[Session] Buffer cleared; ready for next read";
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
                            boost::bind(&Session::handle_read, this,
                                        boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred));
}