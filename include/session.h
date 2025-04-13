#ifndef SESSION_H
#define SESSION_H

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <string>

using boost::asio::ip::tcp;

class Session
{
public:
    Session(boost::asio::io_service &io_service);
    ~Session();

    tcp::socket &socket();
    void start();

private:
    void handle_read(const boost::system::error_code &error,
                     size_t bytes_transferred);
    void handle_write(const boost::system::error_code &error);

    tcp::socket socket_;
    enum
    {
        max_length = 8192
    };
    char data_[max_length];
    std::string request_buffer_;
};

#endif // SESSION_H