///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2014 MaidSafe.net Limited
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
///////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <memory>
#include <functional>
#include <vector>
#include <boost/asio.hpp>
#include <maidsafe/crux/acceptor.hpp>
#include <maidsafe/crux/socket.hpp>

class session : public std::enable_shared_from_this<session>
{
    using message_type = std::vector<char>;

public:
    session(std::shared_ptr<maidsafe::crux::socket> socket)
        : socket(socket)
    {
    }

    void start()
    {
        do_receive();
    }

private:
    void do_receive()
    {
        assert(socket);
        auto self(shared_from_this());
        std::shared_ptr<message_type> message = std::make_shared<message_type>(1400);
        socket->async_receive(boost::asio::buffer(*message),
                              [self, message] (boost::system::error_code error,
                                               std::size_t length)
                              {
                                  if (!error)
                                  {
                                      // Echo the message
                                      self->do_send(message, length);
                                  }
                              });
    }

    void do_send(std::shared_ptr<message_type> message,
                 std::size_t length)
    {
        assert(socket);
        auto self(shared_from_this());
        socket->async_send(boost::asio::buffer(*message, length),
                           [self, message] (boost::system::error_code error,
                                            std::size_t)
                           {
                               if (!error)
                               {
                                   self->do_receive();
                               }
                           });
    }

private:
    std::shared_ptr<maidsafe::crux::socket> socket;
};

class server
{
public:
    server(boost::asio::io_service& io,
           const boost::asio::ip::udp::endpoint& local_endpoint)
        : acceptor(io, local_endpoint)
    {
        do_accept();
    }

    ~server()
    {
    }

private:
    void do_accept()
    {
        auto socket = std::make_shared<maidsafe::crux::socket>(acceptor.get_io_service());
        acceptor.async_accept(*socket,
                              [this, socket] (boost::system::error_code error)
                              {
                                  if (!error)
                                  {
                                      // Session will keep itself alive as long as required
                                      std::make_shared<session>(socket)->start();
                                      do_accept();
                                  }
                              });
    }

private:
    maidsafe::crux::acceptor acceptor;
};

int main(int argc, char *argv[])
{
    if (argc <= 1)
        return 1;

    auto server_port = std::stoi(argv[1]);

    boost::asio::io_service io;
    boost::asio::ip::udp::endpoint endpoint(boost::asio::ip::udp::v4(), server_port);
    server s(io, endpoint);
    io.run();
    return 0;
}
