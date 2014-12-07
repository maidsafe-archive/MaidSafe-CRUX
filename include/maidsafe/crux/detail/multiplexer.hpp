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

#include <boost/asio/placeholders.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/udp.hpp>

#include <maidsafe/crux/detail/buffer.hpp>
#include <maidsafe/crux/detail/header.hpp>
#include <maidsafe/crux/detail/sequence_number.hpp>

namespace maidsafe
{
namespace crux
{
namespace detail
{

class socket_base;

// FIXME: Thread-safety (strand?)

class multiplexer : public std::enable_shared_from_this<multiplexer>
{
    static const size_t header_size = std::tuple_size<header_data_type>::value;

public:
    using protocol_type = boost::asio::ip::udp;
    using next_layer_type = protocol_type::socket;
    using endpoint_type = protocol_type::endpoint;
    using buffer_type = detail::buffer;
    using sequence_number_type = detail::sequence_number<std::uint32_t>;

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
              typename CompletionToken>
    typename boost::asio::async_result<
        typename boost::asio::handler_type<CompletionToken,
                                           void(boost::system::error_code, std::size_t)>::type
        >::type
    async_send_to(ConstBufferSequence&& buffers,
                  const endpoint_type& endpoint,
                  CompletionToken&& token);

    template <typename ConnectHandler>
    void send_handshake(const endpoint_type& remote_endpoint,
                        sequence_number_type initial,
                        std::size_t retransmission_count,
                        ConnectHandler&& handler);

    void start_receive();

    next_layer_type& next_layer();
    const next_layer_type& next_layer() const;

private:
    multiplexer(next_layer_type&& udp_socket);

    void do_start_receive();

    void process_peek(boost::system::error_code, endpoint_type);

    template <typename AcceptHandler>
    void process_accept(const boost::system::error_code& error,
                        socket_base *,
                        std::shared_ptr<buffer_type> datagram,
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

    if (receive_calls++ == 0)
    {
        do_start_receive();
    }
}

template <typename AcceptHandler>
void multiplexer::process_accept(const boost::system::error_code& error,
                                 socket_base *socket,
                                 std::shared_ptr<buffer_type> payload,
                                 const endpoint_type& current_remote_endpoint,
                                 AcceptHandler&& handler)
{
    if (!error)
    {
        socket->remote_endpoint(current_remote_endpoint);
        // Queue payload for later use
        socket->process_data(error, payload->size(), payload);
    }
    handler(error);
}

template <typename ConnectHandler>
void multiplexer::send_handshake(const endpoint_type& remote_endpoint,
                                 sequence_number_type initial,
                                 std::size_t retransmission_count,
                                 ConnectHandler&& handler)
{
    auto handshake = std::make_shared<header_data_type>();
    detail::encoder encoder(handshake->data(), handshake->size());
    encoder.put<std::uint16_t>(constant::header::type_handshake
                               | std::min<std::size_t>(3, retransmission_count));
    encoder.put<std::uint16_t>(constant::header::version);
    encoder.put<std::uint32_t>(initial.value());
    encoder.put<std::uint16_t>(0); // FIXME: Ack
    next_layer().async_send_to
        (boost::asio::buffer(*handshake),
         remote_endpoint,
         [handler, handshake] (boost::system::error_code error, std::size_t length) mutable
         {
             assert(length == handshake->size());
             handler(error);
         });
}

template <typename ConstBufferSequence,
          typename CompletionToken>
typename boost::asio::async_result<
    typename boost::asio::handler_type<CompletionToken,
                                       void(boost::system::error_code, std::size_t)>::type
    >::type
multiplexer::async_send_to(ConstBufferSequence&& buffers,
                           const endpoint_type& endpoint,
                           CompletionToken&& token)
{
    namespace asio = boost::asio;
    using boost::system::error_code;

    static header_data_type dummy_header;

    // FIXME: Congestion control
    typename asio::handler_type<CompletionToken,
                       void(boost::system::error_code, std::size_t)>::type handler(std::forward<CompletionToken>(token));

    boost::asio::async_result<decltype(handler)> result(handler);
    next_layer().async_send_to
        (concatenate( asio::buffer(dummy_header)
                      , std::forward<ConstBufferSequence>(buffers)),
         endpoint,
         [handler](const error_code& error, std::size_t size) mutable {
            handler( error
                     , (size >= header_size) ? size - header_size
                     : 0);
        });
    return result.get();
}

inline void multiplexer::start_receive()
{
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

    // FIXME: Handle error == operation_aborted
    // FIXME: Parse datagram (and only enqueue payload packets)

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
        auto payload = std::make_shared<buffer_type>(payload_size);

        next_layer().receive_from
            ( concatenate(asio::buffer(header_data), asio::buffer(*payload))
            , remote_endpoint
            , next_layer_type::message_flags()
            , error);

        // Unknown endpoint
        if (!acceptor_queue.empty())
        {
            auto input = std::move(acceptor_queue.front());
            acceptor_queue.pop();
            process_accept(error,
                           std::get<0>(*input),
                           payload,
                           remote_endpoint,
                           std::get<1>(*input));
        }
        // FIXME: else enqueue or ignore datagram?
        // FIXME: Collect datagrams from different remote_endpoints
        // FIXME: Call start_receive()?
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

        crux_socket.process_data(error, payload_size, payload);
    }

    if (--receive_calls  > 0)
    {
        do_start_receive();
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
