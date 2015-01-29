///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2014 MaidSafe.net Limited
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
///////////////////////////////////////////////////////////////////////////////

#ifndef MAIDSAFE_CRUX_DETAIL_MULTIPLEXER_HPP
#define MAIDSAFE_CRUX_DETAIL_MULTIPLEXER_HPP

#include <atomic>
#include <memory>
#include <functional>
#include <queue>
#include <map>
#include <queue>
#include <tuple>

#include <boost/optional.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/udp.hpp>

#include <maidsafe/crux/detail/buffer.hpp>
#include <maidsafe/crux/detail/header.hpp>
#include <maidsafe/crux/detail/socket_base.hpp>

namespace maidsafe
{
namespace crux
{
namespace detail
{

class decoder;

// FIXME: Thread-safety (strand?)

class multiplexer : public std::enable_shared_from_this<multiplexer>
{
    static const size_t header_size = std::tuple_size<header_data_type>::value;

public:
    using protocol_type = boost::asio::ip::udp;
    using next_layer_type = protocol_type::socket;
    using endpoint_type = protocol_type::endpoint;
    using buffer_type = detail::buffer;
    using sequence_type = socket_base::sequence_type;
    using ack_sequence_type = socket_base::ack_sequence_type;

    template <typename... Types>
    static std::shared_ptr<multiplexer> create(Types&&...);

    ~multiplexer();

    void add(socket_base *);
    void remove(socket_base *);

    template <typename SocketType,
              typename AcceptHandler>
    void async_accept(SocketType&,
                      AcceptHandler&& handler);

    template <typename ConstBufferSequence,
              typename WriteHandler>
    void send_data(ConstBufferSequence&& buffers,
                   const endpoint_type& endpoint,
                   sequence_type sequence,
                   boost::optional<ack_sequence_type> ack,
                   std::size_t retransmission_count,
                   WriteHandler&& handler);

    template <typename ConnectHandler>
    void send_handshake(const endpoint_type& remote_endpoint,
                        sequence_type initial,
                        boost::optional<ack_sequence_type> ack,
                        std::size_t retransmission_count,
                        ConnectHandler&& handler);

    template <typename ConnectHandler>
    void send_keepalive(const endpoint_type& remote_endpoint,
                        sequence_type sequence,
                        boost::optional<ack_sequence_type> ack,
                        std::size_t retransmission_count,
                        ConnectHandler&& handler);

    void start_receive();

    next_layer_type& next_layer();
    const next_layer_type& next_layer() const;

private:
    multiplexer(next_layer_type&& udp_socket);

    void do_start_receive();

    void process_peek(boost::system::error_code, endpoint_type);

    void establish_connection(std::size_t, endpoint_type);

    void process_handshake(socket_base&, endpoint_type, std::uint16_t, detail::decoder&);
    void process_keepalive(socket_base&, std::uint16_t, detail::decoder&);
    void process_data(socket_base&,
                      std::uint16_t,
                      detail::decoder&,
                      const boost::system::error_code&,
                      std::size_t,
                      std::shared_ptr<buffer_type>);

    template <typename AcceptHandler>
    void process_accept(const boost::system::error_code& error,
                        socket_base *,
                        const endpoint_type& remote_endpoint,
                        AcceptHandler&& handler);

private:
    next_layer_type udp_socket;

    using socket_map = std::map<endpoint_type, socket_base *>;
    socket_map sockets;

    std::atomic<int> receive_calls;

    // FIXME: Move to acceptor class
    // FIXME: Bounded queue with pending accept requests? (like listen() backlog)
    using accept_handler_type = std::function<void (const boost::system::error_code&)>;
    using accept_input_type = std::tuple<socket_base *, accept_handler_type>;
    std::queue<std::unique_ptr<accept_input_type>> acceptor_queue;

    endpoint_type next_remote_endpoint;
};

} // namespace detail
} // namespace crux
} // namespace maidsafe

#include <cassert>
#include <algorithm>
#include <utility>
#include <boost/asio/buffer.hpp>
#include <maidsafe/crux/detail/socket_base.hpp>
#include <maidsafe/crux/detail/concatenate.hpp>
#include <maidsafe/crux/detail/encoder.hpp>
#include <maidsafe/crux/detail/decoder.hpp>

namespace maidsafe
{
namespace crux
{
namespace detail
{

template <typename... Types>
std::shared_ptr<multiplexer> multiplexer::create(Types&&... args)
{
    std::shared_ptr<multiplexer> self(new multiplexer{std::forward<Types>(args)...});
    return self;
}

inline multiplexer::multiplexer(next_layer_type&& udp_socket)
    : udp_socket(std::move(udp_socket))
    , receive_calls(0)
{
}

inline multiplexer::~multiplexer()
{
    assert(sockets.empty());

    // FIXME: Clean up
}

inline void multiplexer::add(socket_base *socket)
{
    assert(socket);

    sockets.insert(socket_map::value_type(socket->remote_endpoint(), socket));
}

inline void multiplexer::remove(socket_base *socket)
{
    assert(socket);

    sockets.erase(socket->remote_endpoint());
    // FIXME: Prune request queues
}

template <typename SocketType,
          typename AcceptHandler>
void multiplexer::async_accept(SocketType& socket,
                               AcceptHandler&& handler)
{
    std::unique_ptr<accept_input_type> operation(new accept_input_type(&socket,
                                                                       std::move(handler)));
    acceptor_queue.emplace(std::move(operation));

    std::cout << this << " receive_calls -> " << (receive_calls+1) << " " << __LINE__ << "\n";
    if (receive_calls++ == 0)
    {
        do_start_receive();
    }
}

template <typename AcceptHandler>
void multiplexer::process_accept(const boost::system::error_code& error,
                                 socket_base *socket,
                                 const endpoint_type& current_remote_endpoint,
                                 AcceptHandler&& handler)
{
    handler(error);
}

template <typename ConnectHandler>
void multiplexer::send_handshake(const endpoint_type& remote_endpoint,
                                 sequence_type initial,
                                 boost::optional<ack_sequence_type> ack,
                                 std::size_t retransmission_count,
                                 ConnectHandler&& handler)
{
    auto header = std::make_shared<header_data_type>();
    detail::encoder encoder(header->data(), header->size());
    encoder.put<std::uint16_t>(constant::header::type_handshake
                               | std::min<std::size_t>(3, retransmission_count)
                               | (ack ? constant::header::ack_type_cumulative : constant::header::ack_type_none));
    encoder.put<std::uint16_t>(constant::header::version);
    encoder.put<std::uint32_t>(initial.value());
    encoder.put<std::uint32_t>(ack ? ack->value() : 0);
    next_layer().async_send_to
        (boost::asio::buffer(*header),
         remote_endpoint,
         [handler, header] (boost::system::error_code error, std::size_t length) mutable
         {
             assert(length == header->size());
             handler(error);
         });
}

template <typename ConnectHandler>
void multiplexer::send_keepalive(const endpoint_type& remote_endpoint,
                                 sequence_type sequence,
                                 boost::optional<ack_sequence_type> ack,
                                 std::size_t retransmission_count,
                                 ConnectHandler&& handler)
{
    auto header = std::make_shared<header_data_type>();
    detail::encoder encoder(header->data(), header->size());
    encoder.put<std::uint16_t>(constant::header::type_keepalive
                               | std::min<std::size_t>(3, retransmission_count)
                               | (ack ? constant::header::ack_type_cumulative : constant::header::ack_type_none));
    encoder.put<std::uint16_t>(0);
    encoder.put<std::uint32_t>(sequence.value());
    encoder.put<std::uint32_t>(ack ? ack->value() : 0);
    next_layer().async_send_to
        (boost::asio::buffer(*header),
         remote_endpoint,
         [handler, header] (boost::system::error_code error, std::size_t length) mutable
         {
             assert(length == header->size());
             handler(error);
         });
}

template <typename ConstBufferSequence,
          typename WriteHandler>
void multiplexer::send_data(ConstBufferSequence&& buffers,
                            const endpoint_type& endpoint,
                            sequence_type sequence,
                            boost::optional<ack_sequence_type> ack,
                            std::size_t retransmission_count,
                            WriteHandler&& handler)
{
    // FIXME: Congestion control

    auto header = std::make_shared<header_data_type>();
    detail::encoder encoder(header->data(), header->size());
    encoder.put<std::uint16_t>(constant::header::type_data
                               | std::min<std::size_t>(3, retransmission_count)
                               | (ack ? constant::header::ack_type_cumulative : constant::header::ack_type_none));
    encoder.put<std::uint16_t>(0);
    encoder.put<std::uint32_t>(sequence.value());
    encoder.put<std::uint32_t>(ack ? ack->value() : 0);
    next_layer().async_send_to
        (concatenate(boost::asio::buffer(*header),
                     std::forward<ConstBufferSequence>(buffers)),
         endpoint,
         [handler, header](const boost::system::error_code& error, std::size_t size) mutable
         {
             const auto bytes_transferred = (size >= header_size) ? size - header_size : 0;
             handler(error, bytes_transferred);
        });
}

inline void multiplexer::start_receive()
{
    std::cout << this << " receive_calls -> " << (receive_calls+1) << " in start_receive\n";
    if (receive_calls++ == 0)
    {
        do_start_receive();
    }
}

inline void multiplexer::do_start_receive()
{
    auto self(shared_from_this());

    // We need to read with at least one zero sized buffer to
    // get the remote_endpoint information.
    next_layer().async_receive_from
        (boost::asio::buffer(static_cast<char*>(nullptr), 0),
         next_remote_endpoint,
         std::remove_reference<decltype(next_layer())>::type::message_peek,
         [self]
         (boost::system::error_code error, std::size_t size) mutable
         {
            // The size parameter is useless here because what we get
            // is min(buffer_size, datagram_size) and our buffer size is 0.
            self->process_peek(error, self->next_remote_endpoint);
         });
}

inline
void multiplexer::process_peek(boost::system::error_code error,
                               endpoint_type remote_endpoint)
{
    namespace asio = boost::asio;

    switch (error.value())
    {
    case 0:
        // Continue below
        break;

    case boost::asio::error::operation_aborted:
        return;

    default:
        std::cout << this << " receive_calls -> " << (receive_calls-1) << " process_peek begin\n";
        if (--receive_calls > 0)
        {
            do_start_receive();
        }
        return;
    }

    auto recipient = sockets.find(remote_endpoint);

    next_layer_type::bytes_readable command(true);
    next_layer().io_control(command);
    std::size_t datagram_size = command.get();

    if (datagram_size < header_size) {
        // Corrupted packet or someone is being silly.
        return;
    }

    header_data_type header_data;
    std::size_t payload_size = datagram_size - header_size;

    // FIXME: gather-read (header, body)
    // FIXME: Make socket.receive_from commands async.
    if (recipient == sockets.end())
    {
        establish_connection(payload_size, remote_endpoint);
    }
    else
    {
        auto& crux_socket  = *(*recipient).second;
        auto* recv_buffers = crux_socket.get_recv_buffers();

        std::shared_ptr<buffer_type> payload;

        if (recv_buffers) {
            next_layer().receive_from
                ( concatenate( asio::buffer(header_data)
                               , std::move(*recv_buffers))
                  , remote_endpoint
                  , next_layer_type::message_flags()
                  , error );
        }
        else {
            payload = std::make_shared<buffer_type>(payload_size);

            next_layer().receive_from
                ( concatenate( asio::buffer(header_data)
                               , asio::buffer(*payload))
                  , remote_endpoint
                  , next_layer_type::message_flags()
                  , error );
        }

        detail::decoder decoder(header_data.data(), header_data.data() + header_data.size());
        auto type = decoder.get<std::uint16_t>();
        switch (type & constant::header::mask_type)
        {
        case constant::header::type_handshake:
            process_handshake(crux_socket, remote_endpoint, type, decoder);
            break;

        case constant::header::type_keepalive:
            process_keepalive(crux_socket, type, decoder);
            break;

        case constant::header::type_data:
            process_data(crux_socket, type, decoder, error, payload_size, payload);
            break;

        default:
            assert(false);
            break;
        }
    }

    std::cout << this << " receive_calls -> " << (receive_calls-1) << " process_peek end\n";
    if (--receive_calls  > 0)
    {
        do_start_receive();
    }
}

inline
void multiplexer::establish_connection(std::size_t payload_size,
                                       endpoint_type remote_endpoint)
{
    detail::header_data_type header_data;
    auto payload = std::make_shared<buffer_type>(payload_size);

    boost::system::error_code error;
    auto size = next_layer().receive_from(concatenate(boost::asio::buffer(header_data),
                                                      boost::asio::buffer(*payload)),
                                          remote_endpoint,
                                          next_layer_type::message_flags(),
                                          error);
    if (error)
    {
        // Ignore errors on new connections
        return;
    }
    if (size < header_data.size())
    {
        // Ignore datagrams with incomplete header
        return;
    }

    if (acceptor_queue.empty())
    {
        // Ignore handshakes that we did not expect.
        // FIXME: Should we enqueue the most recent requests?
        return;
    }

    auto& input = acceptor_queue.front();
    auto socket = std::get<0>(*input);

    detail::decoder decoder(header_data.data(), header_data.data() + header_data.size());
    auto type = decoder.get<std::uint16_t>();
    switch (type & constant::header::mask_type)
    {
    case constant::header::type_handshake:
        process_handshake(*socket, remote_endpoint, type, decoder);
        break;

    case constant::header::type_keepalive:
        // FIXME: Detect denial-of-service attacks
        process_keepalive(*socket, type, decoder);
        break;

    default:
        // Other packets from unknown remote endpoints may arrive because:
        //   1) The initial handshake was lost in transmission. The handshake
        //      will be retransmitted later, and so will any packet we ignore.
        //   2) Denial-of-service attack, which we will ignore.
        return;
    }

    if (socket->state() == socket_base::connectivity::established)
    {
        boost::system::error_code success;
        auto input = std::move(acceptor_queue.front());
        acceptor_queue.pop();
        process_accept(success,
                       std::get<0>(*input),
                       remote_endpoint,
                       std::get<1>(*input));
    }
}

inline
void multiplexer::process_handshake(socket_base& socket,
                                    endpoint_type remote_endpoint,
                                    std::uint16_t type,
                                    detail::decoder& decoder)
{
    assert((type & constant::header::mask_type) == constant::header::type_handshake);

    auto version = decoder.get<std::uint16_t>();
    sequence_type initial_sequence_number(decoder.get<std::uint32_t>());
    socket.process_handshake(initial_sequence_number, remote_endpoint);

    if (type & constant::header::mask_ack)
    {
        sequence_type ack(decoder.get<std::uint32_t>());
        socket.process_acknowledgement(ack);
    }
}

inline
void multiplexer::process_keepalive(socket_base& socket,
                                    std::uint16_t type,
                                    detail::decoder& decoder)
{
    assert((type & constant::header::mask_type) == constant::header::type_keepalive);

    auto dummy = decoder.get<std::uint16_t>();
    sequence_type sequence_number(decoder.get<std::uint32_t>());
    if (type & constant::header::mask_ack)
    {
        sequence_type ack(decoder.get<std::uint32_t>());
        socket.process_acknowledgement(ack);
    }
}

inline
void multiplexer::process_data(socket_base& socket,
                               std::uint16_t type,
                               detail::decoder& decoder,
                               const boost::system::error_code& error,
                               std::size_t payload_size,
                               std::shared_ptr<buffer_type> payload)
{
    assert((type & constant::header::mask_type) == constant::header::type_data);

    auto dummy = decoder.get<std::uint16_t>();
    sequence_type sequence_number(decoder.get<std::uint32_t>());
    socket.process_data(error, payload_size, payload, sequence_number);

    if (type & constant::header::mask_ack)
    {
        sequence_type ack(decoder.get<std::uint32_t>());
        socket.process_acknowledgement(ack);
    }
}

inline multiplexer::next_layer_type& multiplexer::next_layer()
{
    return udp_socket;
}

inline const multiplexer::next_layer_type& multiplexer::next_layer() const
{
    return udp_socket;
}

} // namespace detail
} // namespace crux
} // namespace maidsafe

#endif // MAIDSAFE_CRUX_DETAIL_MULTIPLEXER_HPP
