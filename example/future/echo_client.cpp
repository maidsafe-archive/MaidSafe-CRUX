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
#include <thread>
#include <iostream>
#include <boost/asio/buffer.hpp>
#include <boost/asio/use_future.hpp>
#include <maidsafe/crux/socket.hpp>

void send_packets(boost::asio::io_service& io,
                  std::string server_host,
                  std::string server_port)
{
    maidsafe::crux::endpoint local_endpoint(boost::asio::ip::udp::v4(), 0);
    maidsafe::crux::socket socket(io, local_endpoint);

    try
    {
        socket.async_connect(server_host, server_port, boost::asio::use_future).get();
        std::cout << "Connected with Success" << std::endl;

        for (int i = 0; i < 5; ++i)
        {
            std::string input{"alpha"};
            socket.async_send(boost::asio::buffer(input), boost::asio::use_future).get();

            unsigned char output[64];
            auto length = socket.async_receive(boost::asio::buffer(output), boost::asio::use_future).get();
            for (auto i = 0; i < length; ++i)
            {
                std::cout << output[i];
            }
            std::cout << std::endl;
        }
    }
    catch (const std::exception& ex)
    {
        std::cout << "Error: " << ex.what() << std::endl;
    }
}

int main(int argc, char *argv[])
{
    if (argc <= 2)
        return 1;

    std::string server_host(argv[1]);
    std::string server_port(argv[2]);

    boost::asio::io_service io;
    boost::asio::io_service::work work(io);

    std::thread io_thread([&io] { io.run(); });

    send_packets(io, server_host, server_port);
    io.stop();
    io_thread.join();
    return 0;
}
