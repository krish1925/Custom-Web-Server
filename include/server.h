#ifndef SERVER_H
#define SERVER_H

#include "session.h"
#include "config_manager.h"

using boost::asio::ip::tcp;

class Server
{
public:
    Server(boost::asio::io_service &io_service, short port, const ConfigManager &cfg);

    Server(boost::asio::io_service &io_service, short port,
           std::shared_ptr<IRequestHandler> default_handler);

protected:
    virtual void start_accept();
    virtual void handle_accept(Session *s, const boost::system::error_code &ec);

    boost::asio::io_service &io_service_;
    tcp::acceptor acceptor_;

    const ConfigManager *cfg_{nullptr};

    std::shared_ptr<IRequestHandler> default_handler_;
};

#endif // SERVER_H