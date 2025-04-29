/*
#include "server.h"
#include <iostream>

Server::Server(boost::asio::io_service &io_service, short port)
    : io_service_(io_service),
      acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
      response_handler_(std::make_shared<EchoResponseHandler>())
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
    std::cout << "Server starting on port " << port << std::endl;
    start_accept();
}

void Server::start_accept()
{
    Session *new_session = new Session(io_service_, response_handler_);
    acceptor_.async_accept(new_session->socket(),
                           boost::bind(&Server::handle_accept, this, new_session,
                                       boost::asio::placeholders::error));
}

void Server::handle_accept(Session *new_session,
                           const boost::system::error_code &error)
{
    if (!error)
    {
        new_session->start();
    }
    else
    {
        delete new_session;
    }

    start_accept();
}
*/

#include "server.h"
#include <iostream>

/* ---------- 生产构造函数 ---------- */
Server::Server(boost::asio::io_service& io,
               short                    port,
               const ConfigManager&     cfg)
  : io_service_(io),
    acceptor_(io, tcp::endpoint(tcp::v4(), port)),
    cfg_(&cfg)
{
    std::cout << "Server starting on port " << port << std::endl;
    start_accept();
}

/* ---------- （可选）测试用构造函数 ---------- */
Server::Server(boost::asio::io_service&         io,
               short                            port,
               std::shared_ptr<IRequestHandler> handler)
  : io_service_(io),
    acceptor_(io, tcp::endpoint(tcp::v4(), port)),
    default_handler_(std::move(handler))
{
    std::cout << "Server starting on port " << port << std::endl;
    start_accept();
}

/* ---------- 开始异步接受 ---------- */
void Server::start_accept()
{
    Session* s = new Session(io_service_, *cfg_);   // 只有这一条
    acceptor_.async_accept(s->socket(),
        boost::bind(&Server::handle_accept, this, s,
                    boost::asio::placeholders::error));
}

/* ---------- 处理 Accept 结果 ---------- */
void Server::handle_accept(Session* s,
                           const boost::system::error_code& ec)
{
    if (!ec) s->start();
    else     delete s;

    start_accept();   // 继续 accept 下一连接
}

