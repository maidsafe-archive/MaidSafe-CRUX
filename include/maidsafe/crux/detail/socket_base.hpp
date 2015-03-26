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

#include <cstdint>
#include <memory>
#include <queue>
#include <utility>

#include <boost/asio/socket_base.hpp>
#include <boost/asio/ip/udp.hpp>

#include <maidsafe/crux/detail/buffer.hpp>
#include <maidsafe/crux/detail/sequence_number.hpp>

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

    socket_base() : state_value(connectivity::closed) {}
    virtual ~socket_base() {}

    endpoint_type remote_endpoint() const { return remote; }

protected:
    friend class multiplexer;

    using sequence_type = detail::sequence_number<std::uint32_t>;
    using ack_field_type = std::uint16_t;
    using ack_sequence_type = sequence_type;

    enum struct connectivity
    {
        closed,
        listening,
        connecting,
        handshaking,
        established
    };

    connectivity state() const { return state_value; }
    void state(connectivity value) { state_value = value; }

    void remote_endpoint(const endpoint_type& r) { remote = r; }

    virtual std::vector<boost::asio::mutable_buffer>* get_recv_buffers() = 0;

    virtual void process_handshake(sequence_type initial,
                                   endpoint_type remote_endpoint) = 0;

    virtual void process_acknowledgement(const ack_sequence_type& ack) = 0;

    virtual void process_data(const boost::system::error_code&,
                              std::size_t bytes_transferred,
                              std::shared_ptr<detail::buffer>,
                              sequence_type) = 0;

    virtual void process_keepalive(sequence_type) = 0;

    virtual void close() = 0;

protected:
    endpoint_type remote;
    connectivity state_value;
};

}}} // namespace maidsafe::crux::detail

#endif // MAIDSAFE_CRUX_DETAIL_SOCKET_BASE_HPP
