#ifndef SERVER_H
#define SERVER_H
#include <boost/asio.hpp>
#include "session.h"
#include "config_manager.h"
#include <memory>

using boost::asio::ip::tcp;

class Server
{
public:
    Server(boost::asio::io_service& io_service, short port, const ConfigManager& cfg);

    Server(boost::asio::io_service& io_service, short port, std::shared_ptr<IRequestHandler>  default_handler);

protected:
    void start_accept();
    void handle_accept(Session*,
                       const boost::system::error_code&);

    boost::asio::io_service& io_service_;
    tcp::acceptor            acceptor_;

    const ConfigManager* cfg_{nullptr};

    std::shared_ptr<IRequestHandler> default_handler_;
};

#endif // SERVER_H