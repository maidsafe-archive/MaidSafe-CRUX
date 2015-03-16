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
#include <list>
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

class acceptor;

namespace detail
{

class decoder;

// FIXME: Thread-safety (strand?)

class multiplexer : public std::enable_shared_from_this<multiplexer>
{
    static const size_t header_size = std::tuple_size<header::data_type>::value;

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

    template <typename AcceptorType,
              typename SocketType,
              typename AcceptHandler>
    void async_accept(AcceptorType&,
                      SocketType&,
                      AcceptHandler&& handler);

    template <typename ConstBufferSequence,
              typename WriteHandler>
    void send_data(ConstBufferSequence&& buffers,
                   const endpoint_type& endpoint,
                   sequence_type sequence,
                   boost::optional<ack_sequence_type> ack,
                   std::uint16_t retransmission_count,
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
    void stop_receive();

    next_layer_type& next_layer();
    const next_layer_type& next_layer() const;

    void disable_accept_requests_from(acceptor&);

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
                        AcceptHandler&& handler);

    boost::asio::io_service& get_io_service() {
        return udp_socket.get_io_service();
    }

    endpoint_type local_loopback_endpoint() const;

    void discard_message();

private:
    next_layer_type udp_socket;

    using socket_map = std::map<endpoint_type, socket_base *>;
    socket_map sockets;

    std::atomic<std::size_t> receive_calls;

    // FIXME: Move to acceptor class
    // FIXME: Bounded queue with pending accept requests? (like listen() backlog)
    using accept_handler_type = std::function<void (const boost::system::error_code&)>;
    using accept_input_type = std::tuple<acceptor*, socket_base *, accept_handler_type>;
    std::list<std::unique_ptr<accept_input_type>> acceptor_queue;

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
    assert(acceptor_queue.empty());
    assert(receive_calls == 0);

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

    if (sockets.empty()) {
        next_layer().close();
    }
}

template <typename AcceptorType,
          typename SocketType,
          typename AcceptHandler>
void multiplexer::async_accept(AcceptorType& acceptor,
                               SocketType& socket,
                               AcceptHandler&& handler)
{
    std::unique_ptr<accept_input_type> operation(new accept_input_type(&acceptor,
                                                                       &socket,
                                                                       std::move(handler)));
    acceptor_queue.emplace_back(std::move(operation));

    start_receive();
}

inline
void multiplexer::disable_accept_requests_from(acceptor& accept) {
    auto i = acceptor_queue.begin();

    while (i != acceptor_queue.end()) {
        if (std::get<0>(**i) == &accept) {
            auto socket  = std::get<1>(**i);
            auto handler = std::move(std::get<2>(**i));
            get_io_service().post([handler]() {
                    handler(boost::asio::error::operation_aborted);
                    });
            stop_receive();
            socket->close();
            acceptor_queue.erase(i++);
        }
        else {
            ++i;
        }
    }
}

template <typename AcceptHandler>
void multiplexer::process_accept(const boost::system::error_code& error,
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
    auto header = std::make_shared<header::data_type>();
    detail::encoder encoder(header->data(), header->size());
    header::handshake(retransmission_count, initial, ack).encode(encoder);
    next_layer().async_send_to
        (boost::asio::buffer(*header),
         remote_endpoint,
         [handler, header] (boost::system::error_code error, std::size_t length) mutable
         {
             assert(length == header->size());
             static_cast<void>(length);
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
    auto header = std::make_shared<header::data_type>();
    detail::encoder encoder(header->data(), header->size());
    header::keepalive(retransmission_count, sequence, ack).encode(encoder);

    next_layer().async_send_to
        (boost::asio::buffer(*header),
         remote_endpoint,
         [handler, header] (boost::system::error_code error, std::size_t length) mutable
         {
             assert(length == header->size());
             static_cast<void>(length);
             handler(error);
         });
}

template <typename ConstBufferSequence,
          typename WriteHandler>
void multiplexer::send_data(ConstBufferSequence&& buffers,
                            const endpoint_type& endpoint,
                            sequence_type sequence,
                            boost::optional<ack_sequence_type> ack,
                            std::uint16_t retransmission_count,
                            WriteHandler&& handler)
{
    auto header = std::make_shared<header::data_type>();
    detail::encoder encoder(header->data(), header->size());
    header::data(retransmission_count, sequence, ack).encode(encoder);

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

inline multiplexer::endpoint_type multiplexer::local_loopback_endpoint() const {
    namespace ip = boost::asio::ip;
    auto local = next_layer().local_endpoint();

    if (local.address().is_v4()) {
        return endpoint_type(ip::address_v4::loopback(), local.port());
    }
    else {
        return endpoint_type(ip::address_v6::loopback(), local.port());
    }
}

inline void multiplexer::start_receive()
{
    // Each socket and acceptor may invoke only one receive call at a time.
    // Since both, a socket and an acceptor may invoke this function
    // from inside a handler, and since the 'receive_calls' counter gets
    // decreased only after handlers are executed we need to allow an
    // error by 2.
    assert(receive_calls < sockets.size() + acceptor_queue.size() + 2U);

    if (receive_calls++ == 0)
    {
        do_start_receive();
    }
}

inline void multiplexer::stop_receive()
{
    // Each socket may invoke only one receive call at a time.
    assert(receive_calls > 0);
    assert(receive_calls <= sockets.size() + acceptor_queue.size() + 1U);

    if (--receive_calls == 0)
    {
        // The fact that we're here means that the last socket that
        // was receiving just got closed. To prevent the io_service
        // from blocking, we fulfill the last receive request by
        // sending ourself an empty packet.
        auto self = shared_from_this();
        static char empty_buffer_storage;
        next_layer().async_send_to
            ( boost::asio::buffer(&empty_buffer_storage, 1)
            , local_loopback_endpoint()
            , [self](boost::system::error_code, std::size_t) {});
    }
}

inline void multiplexer::do_start_receive()
{
    auto self(shared_from_this());

    // We need to read with at least one non zero sized buffer to
    // get the remote_endpoint information.
    static char empty_buffer_storage;
    next_layer().async_receive_from
        (boost::asio::buffer(&empty_buffer_storage, sizeof(empty_buffer_storage)),
         next_remote_endpoint,
         std::remove_reference<decltype(next_layer())>::type::message_peek,
         [self]
         (boost::system::error_code error, std::size_t /*size*/) mutable
         {
            // The size parameter is useless here because what we get
            // is min(buffer_size, datagram_size) and our buffer size is 0.
            self->process_peek(error, self->next_remote_endpoint);
         });
}

inline void multiplexer::discard_message() {
    boost::system::error_code error;
    endpoint_type remote_endpoint;
    static char empty_buffer_storage;
    next_layer().receive_from
        ( boost::asio::buffer(&empty_buffer_storage, 1)
        , remote_endpoint, next_layer_type::message_flags(), error );
}

inline
void multiplexer::process_peek(boost::system::error_code error,
                               endpoint_type remote_endpoint)
{
    namespace asio = boost::asio;

    if (!next_layer().is_open()) return;

    if (receive_calls == 0) {
        // We've received our own empty message to fullfill the last receive
        // request or we've received someone's else's message while doing so.
        discard_message();
        return;
    }

    assert(receive_calls <= sockets.size() + acceptor_queue.size() + 1U);

    switch (error.value())
    {
    case 0:
        // Continue below
        break;
#if defined(BOOST_ASIO_WINDOWS)
    case ERROR_MORE_DATA:
        // Continue below
        break;
#endif // defined(BOOST_ASIO_WINDOWS)

    case boost::asio::error::operation_aborted:
        discard_message();
        --receive_calls;
        return;

    default:
        // Since we're here a socket must have been receiving
        // and its receive request must be fulfilled, so we
        // must start receiving again.
        discard_message();
        do_start_receive();
        return;
    }

    auto recipient = sockets.find(remote_endpoint);

    next_layer_type::bytes_readable command(true);
    next_layer().io_control(command);
    std::size_t datagram_size = command.get();
#ifdef __FreeBSD__
    // FreeBSD's FIONREAD when asked of a UDP socket includes the UDP and IP headers which
    // for IPv4 is 16 bytes and for IPv6 is 24 bytes
    datagram_size -= remote_endpoint.address().is_v6() ? 24 : 16;
#endif

    if (datagram_size < header_size) {
        // Our empty packet, corrupted packet or someone is being silly.
        discard_message();
        do_start_receive();
        return;
    }

    header::data_type header_data;
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
        switch (type & header::constant::mask_type)
        {
        case header::constant::type_handshake:
            process_handshake(crux_socket, remote_endpoint, type, decoder);
            break;

        case header::constant::type_keepalive:
            process_keepalive(crux_socket, type, decoder);
            break;

        case header::constant::type_data:
            process_data(crux_socket, type, decoder, error, payload_size, payload);
            break;

        default:
            assert(false);
            break;
        }
    }

    if (--receive_calls > 0) {
        do_start_receive();
    }
}

inline
void multiplexer::establish_connection(std::size_t payload_size,
                                       endpoint_type remote_endpoint)
{
    header::data_type header_data;
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
    auto socket = std::get<1>(*input);

    detail::decoder decoder(header_data.data(), header_data.data() + header_data.size());
    auto type = decoder.get<std::uint16_t>();
    switch (type & header::constant::mask_type)
    {
    case header::constant::type_handshake:
        process_handshake(*socket, remote_endpoint, type, decoder);
        break;

    case header::constant::type_keepalive:
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
        acceptor_queue.pop_front();
        process_accept(success,
                       std::get<2>(*input));
        --receive_calls;
    }
    else {
        ++receive_calls;
    }
}

inline
void multiplexer::process_handshake(socket_base& socket,
                                    endpoint_type remote_endpoint,
                                    std::uint16_t type,
                                    detail::decoder& decoder)
{
    header::handshake msg(type, decoder);
    socket.process_handshake(msg.initial_sequence_number, remote_endpoint);

    if (msg.ack)
    {
        socket.process_acknowledgement(*msg.ack);
    }
}

inline
void multiplexer::process_keepalive(socket_base& socket,
                                    std::uint16_t type,
                                    detail::decoder& decoder)
{
    header::keepalive msg(type, decoder);
    socket.process_keepalive(msg.sequence_number);

    if (msg.ack)
    {
        socket.process_acknowledgement(*msg.ack);
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
    header::data msg(type, decoder);
    socket.process_data(error, payload_size, payload, msg.sequence_number);

    if (msg.ack)
    {
        socket.process_acknowledgement(*msg.ack);
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
