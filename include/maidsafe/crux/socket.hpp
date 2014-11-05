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

#include <boost/asio/basic_io_object.hpp>
#include <boost/asio/async_result.hpp>

#include <maidsafe/crux/detail/socket_base.hpp>
#include <maidsafe/crux/detail/service.hpp>
#include <maidsafe/crux/endpoint.hpp>
#include <maidsafe/crux/resolver.hpp>

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
    using service_type = detail::service;
    using resolver_type = crux::resolver;

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
    async_send(const ConstBufferSequence& buffers,
               CompletionToken&& token);

    // Get the io_service associated with the socket
    boost::asio::io_service& get_io_service();

    // Get the local endpoint of the socket
    endpoint_type local_endpoint() const;

private:
    friend class detail::multiplexer;
    friend class acceptor;

    void set_multiplexer(std::shared_ptr<detail::multiplexer> multiplexer);

    virtual void enqueue(const boost::system::error_code& error,
                         std::size_t bytes_transferred,
                         std::shared_ptr<detail::buffer> datagram)
    {
        // FIXME: Thread-safe
        if (receive_input_queue.empty())
        {
            std::unique_ptr<receive_output_type>
                operation(new receive_output_type(error,
                                                  bytes_transferred,
                                                  datagram));
            receive_output_queue.emplace(std::move(operation));
        }
        else
        {
            auto input = std::move(receive_input_queue.front());
            receive_input_queue.pop();

            process_receive(error,
                            bytes_transferred,
                            datagram,
                            std::get<0>(*input),
                            std::get<1>(*input));
        }
    }

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

    template <typename MutableBufferSequence,
              typename ReadHandler>
    void process_receive(const boost::system::error_code& error,
                         std::size_t bytes_transferred,
                         std::shared_ptr<detail::buffer> datagram,
                         const MutableBufferSequence&,
                         ReadHandler&&);

private:
    std::shared_ptr<detail::multiplexer> multiplexer;

    using read_handler_type = std::function<void (const boost::system::error_code&, std::size_t)>;
    using receive_input_type = std::tuple<boost::asio::mutable_buffer, read_handler_type>;
    using receive_output_type = std::tuple<boost::system::error_code, std::size_t, std::shared_ptr<detail::buffer>>;
    std::queue<std::unique_ptr<receive_input_type>> receive_input_queue;
    std::queue<std::unique_ptr<receive_output_type>> receive_output_queue;
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
    : boost::asio::basic_io_object<service_type>(io)
{
}

inline socket::socket(boost::asio::io_service& io,
                      const endpoint_type& local_endpoint)
    : boost::asio::basic_io_object<service_type>(io),
      multiplexer(get_service().add(local_endpoint))
{
}

inline socket::~socket()
{
    if (multiplexer)
    {
        multiplexer->remove(this);
    }
    get_service().remove(local_endpoint());
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
        get_io_service().post
            ([this, remote_endpoint, handler] () mutable
             {
                 boost::system::error_code success;
                 this->process_connect(success,
                                       remote_endpoint,
                                       std::forward<decltype(handler)>(handler));
             });
    }
    return result.get();
}

template <typename ConnectHandler>
void socket::process_connect(const boost::system::error_code& error,
                             const endpoint_type& remote_endpoint,
                             ConnectHandler&& handler)
{
    // FIXME: Remove from multiplexer if already connected to different remote endpoint?
    if (!error)
    {
        assert(multiplexer);
        remote = remote_endpoint;
        multiplexer->add(this);
    }
    handler(error);
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
            std::unique_ptr<receive_input_type> operation(new receive_input_type(buffers, std::move(handler)));
            receive_input_queue.emplace(std::move(operation));

            multiplexer->start_receive();
        }
        else
        {
            get_io_service().post
                ([this, buffers, handler] () mutable
                 {
                     // FIXME: Thread-safe
                     auto output = std::move(this->receive_output_queue.front());
                     this->receive_output_queue.pop();
                     this->process_receive(std::get<0>(*output),
                                           std::get<1>(*output),
                                           std::get<2>(*output),
                                           buffers,
                                           handler);
                 });
        }
    }
    return result.get();
}

template <typename MutableBufferSequence,
          typename ReadHandler>
void socket::process_receive(const boost::system::error_code& error,
                             std::size_t bytes_transferred,
                             std::shared_ptr<detail::buffer> datagram,
                             const MutableBufferSequence& buffers,
                             ReadHandler&& handler)
{
    auto length = std::min(boost::asio::buffer_size(buffers), bytes_transferred);
    if (!error)
    {
        boost::asio::buffer_copy(buffers,
                                 boost::asio::buffer(*datagram),
                                 length);
    }
    handler(error, length);
}

template <typename ConstBufferSequence,
          typename CompletionToken>
typename boost::asio::async_result<
    typename boost::asio::handler_type<CompletionToken,
                                       void(boost::system::error_code, std::size_t)>::type
    >::type
socket::async_send(const ConstBufferSequence& buffers,
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
        multiplexer->async_send_to
            (buffers,
             remote,
             [handler] (const boost::system::error_code& error,
                        std::size_t bytes_transferred) mutable
             {
                 // Process send
                 handler(error, bytes_transferred);
             });
    }
    return result.get();
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
