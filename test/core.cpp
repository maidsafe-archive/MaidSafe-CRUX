///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2014 MaidSafe.net Limited
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
///////////////////////////////////////////////////////////////////////////////

#define BOOST_TEST_MODULE core_tests

#include <boost/test/unit_test.hpp>
#include <boost/system/error_code.hpp>
#include <maidsafe/crux/socket.hpp>
#include <maidsafe/crux/acceptor.hpp>

namespace asio = boost::asio;
using error_code    = boost::system::error_code;
using endpoint_type = boost::asio::ip::udp::endpoint;

// FIXME: Add test for connect/accept only (no data exchange).
// This is currently not possible since we don't have the
// handshake in place yet.

BOOST_AUTO_TEST_CASE(sigle_exchange)
{
    using namespace maidsafe;
    using udp = asio::ip::udp;

    asio::io_service ios;

    crux::socket   client_socket(ios, endpoint_type(udp::v4(), 0));
    crux::socket   server_socket(ios);

    crux::acceptor acceptor(ios, endpoint_type(udp::v4(), 0));

    const std::string message_text = "TEST_MESSAGE";
    std::vector<char>  rx_data(message_text.size());
    std::vector<char>  tx_data(message_text.begin(), message_text.end());

    acceptor.async_accept(server_socket, [&](error_code error) {
            BOOST_ASSERT(!error);

            server_socket.async_receive(
                asio::buffer(rx_data),
                [&](const error_code& error, size_t size) {
                  BOOST_ASSERT(!error);
                  BOOST_ASSERT(size == rx_data.size());
                  BOOST_ASSERT(rx_data == tx_data);
                });
            });

    client_socket.async_connect(
            acceptor.local_endpoint(),
            [&](error_code error) {
              BOOST_ASSERT(!error);

              client_socket.async_send(asio::buffer(tx_data),
                  [&](error_code error, size_t size) {
                    BOOST_REQUIRE(!error);
                    BOOST_REQUIRE_EQUAL(size, tx_data.size());
                  });
            });

    ios.run();
}

BOOST_AUTO_TEST_CASE(double_exchange)
{
    using namespace maidsafe;
    using udp = asio::ip::udp;

    asio::io_service ios;

    crux::socket   client_socket(ios, endpoint_type(udp::v4(), 0));
    crux::socket   server_socket(ios);

    crux::acceptor acceptor(ios, endpoint_type(udp::v4(), 0));

    const std::string  blank_message  = "XXXXXXXXXXXXX";
    std::string        message1_text  = "TEST_MESSAGE1";
    std::string        message2_text  = "TEST_MESSAGE1";

    std::vector<char>  rx_data(blank_message.begin(), blank_message.end());

    acceptor.async_accept(server_socket, [&](error_code error) {
            BOOST_ASSERT(!error);

            server_socket.async_receive(
                asio::buffer(rx_data),
                [&](const error_code& error, size_t size) {
                  BOOST_ASSERT(!error);
                  BOOST_ASSERT(size == message1_text.size());
                  BOOST_REQUIRE_EQUAL
                      ( std::string(rx_data.begin(), rx_data.end())
                      , message1_text);

                  rx_data.assign(blank_message.begin(), blank_message.end());

                  server_socket.async_receive(
                      asio::buffer(rx_data),
                      [&](const error_code& error, size_t size) {
                          BOOST_ASSERT(!error);
                          BOOST_ASSERT(size == rx_data.size());
                          BOOST_REQUIRE_EQUAL
                            ( std::string(rx_data.begin(), rx_data.end())
                            , message2_text);
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

