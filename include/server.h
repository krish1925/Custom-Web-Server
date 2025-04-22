#ifndef SERVER_H
#define SERVER_H

#include <boost/asio.hpp>
#include "session.h"
#include <memory>

using boost::asio::ip::tcp;

class Server
{
public:
    Server(boost::asio::io_service &io_service, short port);
    
    // constructor with response handler injection for testing
    Server(boost::asio::io_service &io_service, short port,
           std::shared_ptr<IResponseHandler> response_handler);

protected:
    // protected for testing
    virtual void start_accept();
    virtual void handle_accept(Session *new_session,
                      const boost::system::error_code &error);

    boost::asio::io_service &io_service_;
    tcp::acceptor acceptor_;
    std::shared_ptr<IResponseHandler> response_handler_;
};

#endif // SERVER_H