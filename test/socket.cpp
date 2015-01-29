///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2014 MaidSafe.net Limited
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
///////////////////////////////////////////////////////////////////////////////

#include <boost/test/unit_test.hpp>
#include <boost/system/error_code.hpp>
#include <maidsafe/crux/socket.hpp>
#include <maidsafe/crux/acceptor.hpp>

namespace asio = boost::asio;
using error_code    = boost::system::error_code;
using endpoint_type = boost::asio::ip::udp::endpoint;

static std::string to_string(const std::vector<char>& v) {
    return std::string(v.begin(), v.end());
}

BOOST_AUTO_TEST_SUITE(socket_suite)

BOOST_AUTO_TEST_CASE(accept_connect)
{
    std::cout << "test " << __LINE__ << "\n";
    using namespace maidsafe;
    using udp = asio::ip::udp;

    asio::io_service ios;

    crux::socket client_socket(ios, endpoint_type(udp::v4(), 0));
    crux::socket server_socket(ios);

    crux::acceptor acceptor(ios, endpoint_type(udp::v4(), 0));

    const std::string message_text = "TEST_MESSAGE";
    std::vector<char>  rx_data(message_text.size());
    std::vector<char>  tx_data(message_text.begin(), message_text.end());

    bool tested_client = false;
    bool tested_server = false;

    acceptor.async_accept(server_socket, [&](error_code error) {
                                           BOOST_ASSERT(!error);
                                           tested_server = true;
                                           std::cout << "ACCEPT\n";
                                         });

    client_socket.async_connect(acceptor.local_endpoint(),
                                [&](error_code error) {
                                  BOOST_ASSERT(!error);
                                  tested_client = true;
                                  std::cout << "CONNECT\n";
                                });

    ios.run();

    BOOST_REQUIRE(tested_client && tested_server);
}

BOOST_AUTO_TEST_CASE(single_send_and_receive)
{
    std::cout << "test " << __LINE__ << "\n";
    using namespace maidsafe;
    using udp = asio::ip::udp;

    asio::io_service ios;

    crux::socket client_socket(ios, endpoint_type(udp::v4(), 0));
    crux::socket server_socket(ios);

    crux::acceptor acceptor(ios, endpoint_type(udp::v4(), 0));

    const std::string message_text = "TEST_MESSAGE";
    std::vector<char>  rx_data(message_text.size());
    std::vector<char>  tx_data(message_text.begin(), message_text.end());

    bool tested_receive = false;
    bool tested_send    = false;

    acceptor.async_accept(server_socket, [&](error_code error) {
            BOOST_ASSERT(!error);
            std::cout << "ACCEPT\n";

            server_socket.async_receive(
                asio::buffer(rx_data),
                [&](const error_code& error, size_t size) {
                  std::cout << "RECEIVE\n";
                  BOOST_ASSERT(!error);
                  BOOST_REQUIRE_EQUAL(size, tx_data.size());
                  BOOST_REQUIRE_EQUAL(to_string(rx_data), to_string(tx_data));

                  tested_receive = true;
                });
            });

    client_socket.async_connect(
            acceptor.local_endpoint(),
            [&](error_code error) {
              std::cout << "CONNECT\n";
              BOOST_ASSERT(!error);

              client_socket.async_send(asio::buffer(tx_data),
                  [&](error_code error, size_t size) {
                    std::cout << "SEND\n";
                    BOOST_REQUIRE(!error);
                    BOOST_REQUIRE_EQUAL(size, tx_data.size());

                    tested_send = true;
                  });
            });

    ios.run();

    BOOST_REQUIRE(tested_receive && tested_send);
}

BOOST_AUTO_TEST_CASE(double_send_and_receive)
{
    std::cout << "test " << __LINE__ << "\n";
    using namespace maidsafe;
    using udp = asio::ip::udp;

    asio::io_service ios;

    crux::socket   client_socket(ios, endpoint_type(udp::v4(), 0));
    crux::socket   server_socket(ios);

    crux::acceptor acceptor(ios, endpoint_type(udp::v4(), 0));

    const std::string  blank_message  = "XXXXXXXXXXXXX";
    std::string        message1_text  = "TEST_MESSAGE1";
    std::string        message2_text  = "TEST_MESSAGE2";

    std::vector<char>  rx_data(blank_message.begin(), blank_message.end());

    acceptor.async_accept(server_socket, [&](error_code error) {
            BOOST_ASSERT(!error);

            server_socket.async_receive(
                asio::buffer(rx_data),
                [&](const error_code& error, size_t size) {
                  BOOST_ASSERT(!error);
                  BOOST_ASSERT(size == message1_text.size());
                  BOOST_REQUIRE_EQUAL
                      (to_string(rx_data), message1_text);

                  rx_data.assign(blank_message.begin(), blank_message.end());

                  server_socket.async_receive(
                      asio::buffer(rx_data),
                      [&](const error_code& error, size_t size) {
                          BOOST_ASSERT(!error);
                          BOOST_REQUIRE_EQUAL(size, rx_data.size());
                          BOOST_REQUIRE_EQUAL
                            (to_string(rx_data), message2_text);
                      });
                });
            });

    std::vector<char>  tx_data(message1_text.begin(), message1_text.end());

    client_socket.async_connect(
            acceptor.local_endpoint(),
            [&](error_code error) {
              BOOST_ASSERT(!error);

              client_socket.async_send(asio::buffer(tx_data),
                  [&](error_code error, size_t size) {
                    BOOST_REQUIRE(!error);
                    BOOST_REQUIRE_EQUAL(size, tx_data.size());

                    tx_data.assign(message2_text.begin(), message2_text.end());

                    client_socket.async_send(asio::buffer(tx_data),
                        [&](error_code error, size_t size) {
                          BOOST_REQUIRE(!error);
                          BOOST_REQUIRE_EQUAL(size, tx_data.size());
                        });
                  });
            });

    ios.run();
}

BOOST_AUTO_TEST_CASE(single_exchange)
{
    std::cout << "test " << __LINE__ << "\n";
    using namespace maidsafe;
    using udp = asio::ip::udp;

    asio::io_service ios;

    crux::socket client_socket(ios, endpoint_type(udp::v4(), 0));
    crux::socket server_socket(ios);

    crux::acceptor acceptor(ios, endpoint_type(udp::v4(), 0));

    const std::string  blank_message = "XXXXXXXXXXXXX";
    std::string        message1_text = "TEST_MESSAGE1";
    std::string        message2_text = "TEST_MESSAGE2";

    std::vector<char> server_rx_data(blank_message.begin(), blank_message.end());
    std::vector<char> server_tx_data(message2_text.begin(), message2_text.end());

    acceptor.async_accept(server_socket, [&](error_code error) {
            BOOST_ASSERT(!error);

            server_socket.async_receive(
                asio::buffer(server_rx_data),
                [&](const error_code& error, size_t size) {
                  BOOST_ASSERT(!error);
                  BOOST_ASSERT(size == message1_text.size());
                  BOOST_REQUIRE_EQUAL
                      (to_string(server_rx_data), message1_text);

                  server_socket.async_send(
                      asio::buffer(server_tx_data),
                      [&](const error_code& error, size_t size) {
                          BOOST_ASSERT(!error);
                          BOOST_ASSERT(size == server_tx_data.size());
                      });
                });
            });

    std::vector<char> client_rx_data(blank_message.begin(), blank_message.end());
    std::vector<char> client_tx_data(message1_text.begin(), message1_text.end());

    client_socket.async_connect(
            acceptor.local_endpoint(),
            [&](error_code error) {
              BOOST_ASSERT(!error);

              client_socket.async_send(asio::buffer(client_tx_data),
                  [&](error_code error, size_t size) {
                      BOOST_REQUIRE(!error);
                      BOOST_REQUIRE_EQUAL(size, client_tx_data.size());

                      client_socket.async_receive(asio::buffer(client_rx_data),
                          [&](error_code error, size_t size) {
                            BOOST_REQUIRE(!error);
                            BOOST_REQUIRE_EQUAL(size, message2_text.size());

                            BOOST_REQUIRE_EQUAL
                                (to_string(client_rx_data), message2_text);
                      });
               });
           });

    ios.run();
}

BOOST_AUTO_TEST_SUITE_END()
