#include "server.h"
#include <boost/bind.hpp>

Server::Server(boost::asio::io_service &io,
               short port,
               const ConfigManager &cfg)
    : io_service_(io),
      acceptor_(io, tcp::endpoint(tcp::v4(), port)),
      cfg_(&cfg)
{
    std::cout << "Server starting on port " << port << std::endl;
    start_accept();
}

Server::Server(boost::asio::io_service &io,
               short port,
               std::shared_ptr<IRequestHandler> handler)
    : io_service_(io),
      acceptor_(io, tcp::endpoint(tcp::v4(), port)),
      default_handler_(std::move(handler))
{
    std::cout << "Server starting on port " << port << std::endl;
    start_accept();
}

Server::Server(boost::asio::io_service &io_service, short port,
               std::shared_ptr<IResponseHandler> response_handler)
    : io_service_(io_service),
      acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
      response_handler_(response_handler)
{
    start_accept();
}

void Server::start_accept()
{
    Session *s = new Session(io_service_, *cfg_);
    acceptor_.async_accept(s->socket(),
                           boost::bind(&Server::handle_accept, this, s,
                                       boost::asio::placeholders::error));
}

void Server::handle_accept(Session *s,
                           const boost::system::error_code &ec)
{
    if (!ec)
        s->start();
    else
        delete s;

    start_accept();
}
