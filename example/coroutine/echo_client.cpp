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
#include <functional>
#include <boost/asio/spawn.hpp>
#include <maidsafe/crux/socket.hpp>

int main(int argc, char *argv[])
{
    if (argc <= 2)
        return 1;

    std::string server_host(argv[1]);
    std::string server_port(argv[2]);

    boost::asio::io_service io;

    boost::asio::spawn
        (io,
         [&io, server_host, server_port] (boost::asio::yield_context yield)
         {
             maidsafe::crux::endpoint local_endpoint(boost::asio::ip::udp::v4(), 0);
             maidsafe::crux::socket socket(io, local_endpoint);

             boost::system::error_code error;
             socket.async_connect(server_host, server_port, yield[error]);
             if (!error)
             {
                 for (int i = 0; i < 5; ++i)
                 {
                     std::string input{"alpha"};
                     socket.async_send(boost::asio::buffer(input), yield);

                     unsigned char output[64];
                     auto length = socket.async_receive(boost::asio::buffer(output), yield);
                     for (auto i = 0; i < length; ++i)
                     {
                         std::cout << output[i];
                     }
                     std::cout << std::endl;
                 }
             }
         });

    io.run();
    return 0;
}
