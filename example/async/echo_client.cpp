///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2014 MaidSafe.net Limited
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
///////////////////////////////////////////////////////////////////////////////

#include <string>
#include <vector>
#include <functional>
#include <iostream>
#include <boost/asio/connect.hpp>
#include <maidsafe/crux/socket.hpp>

class client
{
    using message_type = std::vector<char>;

public:
    client(boost::asio::io_service& io,
           const std::string& host,
           const std::string& service)
        : socket(io,
                 boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 0)),
          counter(4)
    {
        socket.async_connect(host,
                             service,
                             std::bind(&client::on_connect,
                                       this,
                                       std::placeholders::_1));
    }

private:
    void on_connect(const boost::system::error_code& error)
    {
        std::cout << "Connected with " << error.message() << std::endl;

        if (!error)
        {
            do_send("alpha");
        }
    }

    void do_send(const std::string& data)
    {
        std::shared_ptr<message_type> message = std::make_shared<message_type>(data.begin(), data.end());
        socket.async_send(boost::asio::buffer(*message),
                          [this, message] (boost::system::error_code error,
                                           std::size_t /*length*/)
                          {
                              if (!error)
                              {
                                  do_receive();
                                  if (--counter > 0)
                                  {
                                      do_send("bravo");
                                  }
                              }
                          });
    }

    void do_receive()
    {
        std::shared_ptr<message_type> message = std::make_shared<message_type>(1400);
        socket.async_receive(boost::asio::buffer(*message),
                             [this, message] (boost::system::error_code /*error*/,
                                              std::size_t /*length*/)
                             {
                                 for (auto ch : *message)
                                 {
                                     std::cout << ch;
                                 }
                                 std::cout << std::endl;
                             });
    }

private:
    maidsafe::crux::socket socket;
    int counter;
};

int main(int argc, char *argv[])
{
    if (argc <= 2)
        return 1;

    std::string server_host(argv[1]);
    std::string server_port(argv[2]);

    boost::asio::io_service io;
    client c(io, server_host, server_port);
    io.run();
    return 0;
}
