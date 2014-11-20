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
#include <boost/asio/buffer.hpp>

#include <maidsafe/crux/detail/socket_base.hpp>
#include <maidsafe/crux/detail/service.hpp>
#include <maidsafe/crux/detail/header.hpp>
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

    static const size_t header_size = std::tuple_size<detail::header_data_type>::value;

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

    std::vector<boost::asio::mutable_buffer>* get_recv_buffers() override {
        if (receive_input_queue.empty()) {
            return nullptr;
        }

        return &receive_input_queue.front()->buffers;
    }

    void enqueue(const boost::system::error_code& error,
                 std::size_t bytes_transferred,
                 std::shared_ptr<detail::buffer> datagram) override
    {
        // FIXME: Thread-safe
        if (receive_input_queue.empty())
        {
            assert(datagram);

            using detail::receive_output_type;

            std::unique_ptr<receive_output_type>
                operation(new receive_output_type({ error
                                                  , bytes_transferred
                                                  , datagram }));

            receive_output_queue.emplace(std::move(operation));
        }
        else
        {
            assert(!datagram);

            auto input = std::move(receive_input_queue.front());
            receive_input_queue.pop();

            process_receive( error
                           , input->header_data
                           , bytes_transferred - header_size
                           , std::move(input->handler));
        }
    }

    void process_receive( const boost::system::error_code& error
                        , const detail::header_data_type&  header_data
                        , std::size_t                      bytes_received
                        , read_handler_type&&              handler);

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
                                          std::size_t bytes_transferred,
                                          std::shared_ptr<detail::buffer> datagram,
                                          const MutableBufferSequence&,
                                          read_handler_type&&);

private:
    std::shared_ptr<detail::multiplexer> multiplexer;

    std::queue<std::unique_ptr<detail::receive_input_type>> receive_input_queue;
    std::queue<std::unique_ptr<detail::receive_output_type>> receive_output_queue;
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
                                                            output->size,
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
        , std::size_t                      bytes_transferred
        , std::shared_ptr<detail::buffer>  datagram
        , const MutableBufferSequence&     user_buffers
        , read_handler_type&&              handler)
{
    using detail::header_data_type;
    namespace asio = boost::asio;

    auto length = std::min( asio::buffer_size(user_buffers) + header_size
                          , bytes_transferred);

    size_t payload_size = 0;

    header_data_type header_data;

    if (!error)
    {
        if (length < header_size) {
            assert(0 && "Corrupted packet or someone is being silly");
            return;
        }

        payload_size = length - header_size;

        asio::buffer_copy(asio::buffer(header_data, header_data.size()),
                          asio::buffer(*datagram),
                          header_size);

        asio::buffer_copy(user_buffers,
                          asio::buffer(*datagram) + header_size,
                          payload_size);
    }

    process_receive(error, header_data, payload_size, std::move(handler));
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
        using const_buffer = boost::asio::const_buffer;

        static detail::header_data_type dummy_header;

        std::vector<const_buffer> buffers_with_header;
        buffers_with_header.emplace_back(const_buffer(&dummy_header.front()
                                                     , dummy_header.size()));

        for (const auto& buffer : buffers) {
            buffers_with_header.emplace_back(buffer);
        }

        multiplexer->async_send_to
            (buffers_with_header,
             remote,
             [handler] (const boost::system::error_code& error,
                        std::size_t bytes_transferred) mutable
             {
                 assert(bytes_transferred >= header_size);

                 // Process send
                 handler(error, bytes_transferred - header_size);
             });
    }
    return result.get();
}

inline
void socket::process_receive( const boost::system::error_code& error
                            , const detail::header_data_type&  header_data
                            , std::size_t                      bytes_received
                            , read_handler_type&&              handler)
{
    handler(error, bytes_received);
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
