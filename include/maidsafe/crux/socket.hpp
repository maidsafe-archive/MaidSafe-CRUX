///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2014 MaidSafe.net Limited
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
///////////////////////////////////////////////////////////////////////////////

#ifndef MAIDSAFE_CRUX_SOCKET_HPP
#define MAIDSAFE_CRUX_SOCKET_HPP

#include <functional>
#include <memory>
#include <tuple>
#include <deque>

#include <boost/optional.hpp>
#include <boost/asio/basic_io_object.hpp>
#include <boost/asio/async_result.hpp>
#include <boost/asio/buffer.hpp>

#include <maidsafe/crux/detail/socket_base.hpp>
#include <maidsafe/crux/detail/service.hpp>
#include <maidsafe/crux/detail/cumulative_set.hpp>
#include <maidsafe/crux/endpoint.hpp>
#include <maidsafe/crux/resolver.hpp>

#include <maidsafe/crux/detail/receive_input_type.hpp>
#include <maidsafe/crux/detail/receive_output_type.hpp>

namespace maidsafe
{
namespace crux
{
namespace detail { class multiplexer; }

class acceptor;

class socket
    : public detail::socket_base,
      public boost::asio::basic_io_object<detail::service>
{
    using service_type      = detail::service;
    using resolver_type     = crux::resolver;
    using read_handler_type = detail::receive_input_type::read_handler_type;

public:
    // Construct a socket
    socket(boost::asio::io_service& io);

    // Construct a socket and bind it to the local endpoint
    socket(boost::asio::io_service& io,
           const endpoint_type& local_endpoint);

    virtual ~socket();

    // Start asynchronous connect to remote endpoint
    template <typename CompletionToken>
    typename boost::asio::async_result<
        typename boost::asio::handler_type<CompletionToken,
                                           void(boost::system::error_code)>::type
        >::type
    async_connect(const endpoint_type& remote_endpoint,
                  CompletionToken&& token);

    template <typename CompletionToken>
    typename boost::asio::async_result<
        typename boost::asio::handler_type<CompletionToken,
                                           void(boost::system::error_code)>::type
        >::type
    async_connect(const std::string& remote_host,
                  const std::string& remote_service,
                  CompletionToken&& token);

    // Start asynchronous receive on a connected socket
    template <typename MutableBufferSequence,
              typename CompletionToken>
    typename boost::asio::async_result<
        typename boost::asio::handler_type<CompletionToken,
                                           void(boost::system::error_code, std::size_t)>::type
        >::type
    async_receive(const MutableBufferSequence& buffers,
                  CompletionToken&& token);

    // Start asynchronous send on a connected socket
    template <typename ConstBufferSequence,
              typename CompletionToken>
    typename boost::asio::async_result<
        typename boost::asio::handler_type<CompletionToken,
                                           void(boost::system::error_code, std::size_t)>::type
        >::type
    async_send(ConstBufferSequence&& buffers, CompletionToken&& token);

    // Get the io_service associated with the socket
    boost::asio::io_service& get_io_service();

    // Get the local endpoint of the socket
    endpoint_type local_endpoint() const;

private:
    friend class detail::multiplexer;
    friend class acceptor;

    void set_multiplexer(std::shared_ptr<detail::multiplexer> multiplexer);

    std::vector<boost::asio::mutable_buffer>* get_recv_buffers() override {
        if (receive_input_queue.empty()) {
            return nullptr;
        }

        return &receive_input_queue.front()->buffers;
    }

    virtual void process_handshake(sequence_type initial,
                                   endpoint_type remote_endpoint) override;
    virtual void process_acknowledgement(const ack_sequence_type& ack) override;
    virtual void process_data(const boost::system::error_code& error,
                              std::size_t payload_size,
                              std::shared_ptr<detail::buffer> payload,
                              sequence_type) override;

    void process_receive( const boost::system::error_code& error
                        , std::size_t                      bytes_received
                        , read_handler_type&&              handler);

    template <typename Handler>
    void send_handshake(endpoint_type remote_endpoint,
                        Handler&& handler);

    template <typename Handler>
    void send_keepalive(endpoint_type remote_endpoint,
                        Handler&& handler);

    template <typename ConstBufferSequence, typename Handler>
    void send_data(endpoint_type remote_endpoint,
                   ConstBufferSequence&&,
                   Handler&& handler);

private:
    template <typename Handler,
              typename ErrorCode>
    void invoke_handler(Handler&& handler,
                        ErrorCode error);

    template <typename Handler,
              typename ErrorCode>
    void invoke_handler(Handler&& handler,
                        ErrorCode error,
                        std::size_t size);

    template <typename ConnectHandler>
    void process_connect(const boost::system::error_code& error,
                         const endpoint_type&,
                         ConnectHandler&& handler);

    template <typename ConnectHandler>
    void async_next_connect(resolver_type::iterator where,
                            std::shared_ptr<resolver_type> resolver,
                            ConnectHandler&& handler);

    template <typename ConnectHandler>
    void process_next_connect(const boost::system::error_code& error,
                              resolver_type::iterator where,
                              std::shared_ptr<resolver_type> resolver,
                              ConnectHandler&& handler);

    template <typename MutableBufferSequence>
    void copy_buffers_and_process_receive(const boost::system::error_code& error,
                                          std::shared_ptr<detail::buffer> datagram,
                                          const MutableBufferSequence&,
                                          read_handler_type&&);

private:
    std::shared_ptr<detail::multiplexer> multiplexer;

    std::queue<std::unique_ptr<detail::receive_input_type>> receive_input_queue;
    std::queue<std::unique_ptr<detail::receive_output_type>> receive_output_queue;

    using connect_handler_type = std::function<void (const boost::system::error_code&)>;
    boost::optional<connect_handler_type> connect_handler;

    sequence_type next_sequence;

    using sequence_history_type = detail::cumulative_set<sequence_type, ack_field_type>;
    sequence_history_type sequence_history;
};

} // namespace crux
} // namespace maidsafe

#include <algorithm>
#include <functional>
#include <boost/asio/error.hpp>
#include <boost/asio/io_service.hpp>
#include <maidsafe/crux/detail/multiplexer.hpp>

namespace maidsafe
{
namespace crux
{

inline socket::socket(boost::asio::io_service& io)
    : boost::asio::basic_io_object<service_type>(io),
      next_sequence(get_service().random())
{
}

inline socket::socket(boost::asio::io_service& io,
                      const endpoint_type& local_endpoint)
    : boost::asio::basic_io_object<service_type>(io),
      multiplexer(get_service().add(local_endpoint)),
      next_sequence(get_service().random())
{
}

inline socket::~socket()
{
    if (multiplexer)
    {
        get_service().remove(local_endpoint());
        multiplexer->remove(this);
    }
}

inline boost::asio::io_service& socket::get_io_service()
{
    return boost::asio::basic_io_object<service_type>::get_io_service();
}

inline socket::endpoint_type socket::local_endpoint() const
{
    assert(multiplexer);

    return multiplexer->next_layer().local_endpoint();
}

template <typename CompletionToken>
typename boost::asio::async_result<
    typename boost::asio::handler_type<CompletionToken,
                                       void(boost::system::error_code)>::type
    >::type
socket::async_connect(const endpoint_type& remote_endpoint,
                      CompletionToken&& token)
{
    using handler_type = typename boost::asio::handler_type<CompletionToken,
                                                            void(boost::system::error_code)>::type;
    handler_type handler(std::forward<decltype(token)>(token));
    boost::asio::async_result<decltype(handler)> result(handler);

    if (!multiplexer)
    {
        // Socket must be bound to a local endpoint
        invoke_handler(std::forward<handler_type>(handler),
                       boost::asio::error::invalid_argument);
    }
    else
    {
        switch (state())
        {
        case connectivity::closed:
            state(connectivity::connecting);
            remote = remote_endpoint;
            multiplexer->add(this);
            send_handshake
                (remote_endpoint,
                 [this, remote_endpoint, handler]
                 (boost::system::error_code error) mutable
                 {
                     this->process_connect(error,
                                           remote_endpoint,
                                           std::forward<handler_type>(handler));
                 });
            break;

        case connectivity::established:
            invoke_handler(std::forward<handler_type>(handler),
                           boost::asio::error::already_connected);
            break;

        default:
            invoke_handler(std::forward<handler_type>(handler),
                           boost::asio::error::already_started);
            break;
        }
    }
    return result.get();
}

template <typename ConnectHandler>
void socket::process_connect(const boost::system::error_code& error,
                             const endpoint_type& remote_endpoint,
                             ConnectHandler&& handler)
{
    if (error)
    {
        handler(error);
        return;
    }
    // FIXME: Start retransmission timer
    switch (state())
    {
    case connectivity::connecting:
        // Wait for handshake which arrives via process_handshake()
        connect_handler = handler;
        multiplexer->start_receive();
        break;

    case connectivity::established:
        // FIXME: Remove from multiplexer if already connected to different remote endpoint?
        handler(error);
        break;

    default:
        assert(false);
        break;
    }
}

template <typename CompletionToken>
typename boost::asio::async_result<
    typename boost::asio::handler_type<CompletionToken,
                                       void(boost::system::error_code)>::type
    >::type
socket::async_connect(const std::string& host,
                      const std::string& service,
                      CompletionToken&& token)
{
    using handler_type = typename boost::asio::handler_type<CompletionToken,
                                                            void(boost::system::error_code)>::type;
    handler_type handler(std::forward<decltype(token)>(token));
    boost::asio::async_result<decltype(handler)> result(handler);

    if (!multiplexer)
    {
        // Socket must be bound to a local endpoint
        invoke_handler(std::forward<decltype(handler)>(handler),
                       boost::asio::error::invalid_argument);
    }
    else
    {
        auto resolver = std::make_shared<resolver_type>(std::ref(get_io_service()));
        resolver_type::query query(host, service);
        resolver->async_resolve
            (query,
             [this, handler, resolver]
             (const boost::system::error_code& error,
              resolver_type::iterator where) mutable
             {
                 // Process resolve
                 if (error)
                 {
                     handler(error);
                 }
                 else
                 {
                     this->async_next_connect(where, resolver, handler);
                 }
             });
    }
    return result.get();
}

template <typename ConnectHandler>
void socket::async_next_connect(resolver_type::iterator where,
                                std::shared_ptr<resolver_type> resolver,
                                ConnectHandler&& handler)
{
    async_connect
        (*where,
         [this, where, resolver, handler]
         (const boost::system::error_code& error) mutable
         {
             this->process_next_connect(error,
                                        where,
                                        resolver,
                                        handler);
         });
}

template <typename ConnectHandler>
void socket::process_next_connect(const boost::system::error_code& error,
                                  resolver_type::iterator where,
                                  std::shared_ptr<resolver_type> resolver,
                                  ConnectHandler&& handler)
{
    if (error)
    {
        ++where;
        if (where == resolver_type::iterator())
        {
            // No addresses left to connect to
            handler(error);
        }
        else
        {
            // Try the next address
            async_next_connect(where, resolver, handler);
        }
    }
    else
    {
        process_connect(error, *where, std::forward<decltype(handler)>(handler));
    }
}

template <typename MutableBufferSequence,
          typename CompletionToken>
typename boost::asio::async_result<
    typename boost::asio::handler_type<CompletionToken,
                                       void(boost::system::error_code, std::size_t)>::type
    >::type
socket::async_receive(const MutableBufferSequence& buffers,
                      CompletionToken&& token)
{
    using handler_type = typename boost::asio::handler_type<CompletionToken,
                                                            void(boost::system::error_code, std::size_t)>::type;
    handler_type handler(std::forward<decltype(token)>(token));
    boost::asio::async_result<decltype(handler)> result(handler);

    if (!multiplexer)
    {
        invoke_handler(std::forward<decltype(handler)>(handler),
                       boost::asio::error::not_connected,
                       0);
    }
    else
    {
        if (receive_output_queue.empty())
        {
            using detail::receive_input_type;

            std::unique_ptr<receive_input_type> operation
                (new receive_input_type(buffers, std::move(handler)));

            receive_input_queue.emplace(std::move(operation));

            multiplexer->start_receive();
        }
        else
        {
            // We already have data in the output queue.
            get_io_service().post
                ([this, buffers, handler] () mutable
                 {
                     // FIXME: Thread-safe
                     auto output = std::move(this->receive_output_queue.front());
                     this->receive_output_queue.pop();
                     this->copy_buffers_and_process_receive(output->error,
                                                            output->data,
                                                            buffers,
                                                            handler);
                 });
        }
    }
    return result.get();
}

template <typename MutableBufferSequence>
void socket::copy_buffers_and_process_receive
        ( const boost::system::error_code& error
        , std::shared_ptr<detail::buffer>  payload
        , const MutableBufferSequence&     user_buffers
        , read_handler_type&&              handler)
{
    namespace asio = boost::asio;

    if (!error)
    {
        asio::buffer_copy(user_buffers, asio::buffer(*payload));
    }

    process_receive(error, payload->size(), std::move(handler));
}

template <typename ConstBufferSequence,
          typename CompletionToken>
typename boost::asio::async_result<
    typename boost::asio::handler_type<CompletionToken,
                                       void(boost::system::error_code, std::size_t)>::type
    >::type
socket::async_send(ConstBufferSequence&& buffers,
                   CompletionToken&& token)
{
    using handler_type = typename boost::asio::handler_type<CompletionToken,
                                                            void(boost::system::error_code, std::size_t)>::type;
    handler_type handler(std::forward<decltype(token)>(token));
    boost::asio::async_result<decltype(handler)> result(handler);

    if (!multiplexer)
    {
        invoke_handler(std::forward<decltype(handler)>(handler),
                       boost::asio::error::not_connected,
                       0);
    }
    else
    {
        send_data
            (remote,
             std::forward<ConstBufferSequence>(buffers),
             [handler] (const boost::system::error_code& error,
                        std::size_t bytes_transferred) mutable
             {
                 // Process send
                 handler(error, bytes_transferred);
             });
    }
    return result.get();
}

inline
void socket::process_receive( const boost::system::error_code& error
                            , std::size_t                      bytes_received
                            , read_handler_type&&              handler)
{
    handler(error, bytes_received);
}

inline
void socket::process_data(const boost::system::error_code& error,
                          std::size_t payload_size,
                          std::shared_ptr<detail::buffer> payload,
                          sequence_type sequence_number)
{
    sequence_history.insert(sequence_number);

    // FIXME: Thread-safe
    if (receive_input_queue.empty())
    {
        assert(payload && payload->size() == payload_size);

        using detail::receive_output_type;

        std::unique_ptr<receive_output_type>
            operation(new receive_output_type({ error, payload }));

        receive_output_queue.emplace(std::move(operation));
    }
    else
    {
        assert(!payload);

        auto input = std::move(receive_input_queue.front());
        receive_input_queue.pop();

        process_receive(error, payload_size, std::move(input->handler));
    }
}

template <typename Handler>
void socket::send_handshake(endpoint_type remote_endpoint,
                            Handler&& handler)
{
    assert(multiplexer);

    multiplexer->send_handshake(remote_endpoint,
                                next_sequence++,
                                sequence_history.front(),
                                0, // FIXME
                                std::forward<decltype(handler)>(handler));
}

template <typename Handler>
void socket::send_keepalive(endpoint_type remote_endpoint,
                            Handler&& handler)
{
    assert(multiplexer);

    multiplexer->send_keepalive(remote_endpoint,
                                next_sequence++,
                                sequence_history.front(),
                                0, // FIXME
                                std::forward<decltype(handler)>(handler));
}

template <typename ConstBufferSequence, typename Handler>
void socket::send_data(endpoint_type remote_endpoint,
                       ConstBufferSequence&& buffers,
                       Handler&& handler)
{
    assert(multiplexer);

    multiplexer->send_data(std::forward<decltype(buffers)>(buffers),
                           remote_endpoint,
                           next_sequence++,
                           sequence_history.front(),
                           0, // FIMXE
                           std::forward<decltype(handler)>(handler));
}

inline
void socket::process_handshake(sequence_type initial,
                               endpoint_type remote_endpoint)
{
    sequence_history.insert(initial);

    switch (state())
    {
    case connectivity::listening:
        send_handshake
            (remote_endpoint,
             [this, remote_endpoint]
             (boost::system::error_code error) mutable
             {
                 if (error)
                 {
                     state(connectivity::closed);
                 }
                 else
                 {
                     state(connectivity::handshaking);
                     remote = remote_endpoint;
                 }
             });
        break;

    case connectivity::connecting:
        state(connectivity::handshaking);
        send_keepalive
            (remote_endpoint,
             [this, remote_endpoint]
             (boost::system::error_code error) mutable
             {
                 state(error ? connectivity::closed : connectivity::established);
                 if (connect_handler)
                 {
                     (*connect_handler)(error);
                 }
             });
        break;

    default:
        // FIXME: If state == handshaking with the same remote_endpoint as before then remote probably crashed and attempts a new connection (allow reuse_address?)
        assert(false);
        break;
    }
}

inline
void socket::process_acknowledgement(const ack_sequence_type& ack)
{
    switch (state())
    {
    case connectivity::established:
        // FIXME: Normal ack handling
        if (connect_handler)
        {
            boost::system::error_code success;
            (*connect_handler)(success);
            connect_handler = boost::none;
        }
        break;

    case connectivity::handshaking:
        // FIXME: Check ack and ackfield
        state(connectivity::established);
        break;

    default:
        assert(false);
        break;
    }
}

template <typename Handler,
          typename ErrorCode>
void socket::invoke_handler(Handler&& handler,
                            ErrorCode error)
{
    assert(error);

    get_io_service().post
        ([handler, error]() mutable
         {
             handler(boost::asio::error::make_error_code(error));
         });
}

template <typename Handler,
          typename ErrorCode>
void socket::invoke_handler(Handler&& handler,
                            ErrorCode error,
                            std::size_t size)
{
    assert(error);

    get_io_service().post
        ([handler, error, size]() mutable
         {
             handler(boost::asio::error::make_error_code(error), size);
         });
}

inline void socket::set_multiplexer(std::shared_ptr<detail::multiplexer> value)
{
    multiplexer = value;
}

} // namespace crux
} // namespace maidsafe

#endif // MAIDSAFE_CRUX_SOCKET_HPP
