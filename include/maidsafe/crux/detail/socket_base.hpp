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

    using read_handler_type   = std::function<void (const boost::system::error_code&, std::size_t)>;
    using receive_input_type  = std::tuple<boost::asio::mutable_buffer, read_handler_type>;
    using receive_output_type = std::tuple<boost::system::error_code, std::size_t, std::shared_ptr<detail::buffer>>;

    void remote_endpoint(const endpoint_type& r) { remote = r; }
    virtual void enqueue(const boost::system::error_code&,
                         std::size_t bytes_transferred,
                         std::shared_ptr<detail::buffer>) = 0;

    virtual std::unique_ptr<receive_input_type> dequeue() = 0;

protected:
    endpoint_type remote;
};

} // namespace detail
} // namespace crux
} // namespace maidsafe

#endif // MAIDSAFE_CRUX_DETAIL_SOCKET_BASE_HPP
