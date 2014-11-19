///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2014 MaidSafe.net Limited
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
///////////////////////////////////////////////////////////////////////////////

#ifndef MAIDSAFE_CRUX_DETAIL_SOCKET_BASE_HPP
#define MAIDSAFE_CRUX_DETAIL_SOCKET_BASE_HPP

#include <memory>
#include <queue>

#include <boost/asio/socket_base.hpp>
#include <boost/asio/ip/udp.hpp>

#include <maidsafe/crux/detail/buffer.hpp>
#include <maidsafe/crux/detail/header.hpp>
#include <maidsafe/crux/detail/receive_input_type.hpp>
#include <maidsafe/crux/detail/receive_output_type.hpp>

namespace maidsafe
{
namespace crux
{
namespace detail
{

class socket_base : public boost::asio::socket_base
{
public:
    using endpoint_type = boost::asio::ip::udp::endpoint;

    virtual ~socket_base() {}

    endpoint_type remote_endpoint() const { return remote; }

protected:
    friend class multiplexer;

    void remote_endpoint(const endpoint_type& r) { remote = r; }

    virtual void enqueue(const boost::system::error_code&,
                         std::size_t bytes_transferred,
                         std::shared_ptr<detail::buffer>) = 0;

    virtual std::unique_ptr<receive_input_type> dequeue() = 0;

    virtual void process_receive( const boost::system::error_code&
                                , const header_data_type&
                                , std::size_t
                                , detail::receive_input_type::read_handler_type&&) = 0;

protected:
    endpoint_type remote;
};

}}} // namespace maidsafe::crux::detail

#endif // MAIDSAFE_CRUX_DETAIL_SOCKET_BASE_HPP
