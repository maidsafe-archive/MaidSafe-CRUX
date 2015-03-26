///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2014 MaidSafe.net Limited
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
///////////////////////////////////////////////////////////////////////////////

#ifndef MAIDSAFE_CRUX_ACCEPTOR_HPP
#define MAIDSAFE_CRUX_ACCEPTOR_HPP

#include <maidsafe/crux/detail/config.hpp>

#include <memory>
#include <boost/asio/io_service.hpp>
#include <maidsafe/crux/detail/service.hpp>
#include <maidsafe/crux/endpoint.hpp>
#include <maidsafe/crux/socket.hpp>

namespace maidsafe
{
namespace crux
{

class acceptor
    : public boost::asio::basic_io_object<detail::service>
{
public:
    using endpoint_type = crux::endpoint;
    using socket_type = crux::socket;

    acceptor(boost::asio::io_service& io,
             endpoint_type local_endpoint);

    template <typename CompletionToken>
    typename boost::asio::async_result<
        typename boost::asio::handler_type<CompletionToken,
                                           void(boost::system::error_code)>::type
        >::type
    async_accept(socket_type& socket,
                 CompletionToken&& token);

    endpoint_type local_endpoint() const;

    ~acceptor();

    void close();

private:
    template <typename Handler,
              typename ErrorCode>
    void invoke_handler(Handler&& handler,
                        ErrorCode error);

    template <typename AcceptHandler>
    void process_accept(const boost::system::error_code& error,
                        socket_type& socket,
                        AcceptHandler&& handler);

private:
    std::shared_ptr<detail::multiplexer> multiplexer;
};

} // namespace crux
} // namespace maidsafe

#include <cassert>
#include <utility>
#include <functional>

namespace maidsafe
{
namespace crux
{

inline acceptor::acceptor(boost::asio::io_service& io,
                          endpoint_type local_endpoint)
    : boost::asio::basic_io_object<detail::service>(io),
      multiplexer(get_service().add(local_endpoint))
{
}

inline acceptor::~acceptor()
{
    close();
}

inline void acceptor::close() {
    if (!multiplexer) return;
    auto m = std::move(multiplexer);
    m->disable_accept_requests_from(*this);
}

template <typename CompletionToken>
typename boost::asio::async_result<
    typename boost::asio::handler_type<CompletionToken,
                                       void(boost::system::error_code)>::type
    >::type
acceptor::async_accept(socket_type& socket,
                       CompletionToken&& token)
{
    using handler_type = typename boost::asio::handler_type<CompletionToken,
                                                            void(boost::system::error_code)>::type;
    handler_type handler(std::forward<decltype(token)>(token));
    boost::asio::async_result<decltype(handler)> result(handler);

    if (!multiplexer)
    {
        invoke_handler(std::forward<handler_type>(handler),
                       boost::asio::error::invalid_argument);
    }
    else
    {
        switch (socket.state())
        {
        case socket_type::connectivity::closed:
            socket.state(socket_type::connectivity::listening);
            socket.set_multiplexer(multiplexer);
            multiplexer->async_accept
                (*this, socket,
                 [this, &socket, handler]
                 (const boost::system::error_code& error) mutable
                 {
                     this->process_accept(error, socket, std::move(handler));
                 });
            break;

        case socket_type::connectivity::established:
            invoke_handler(std::forward<decltype(handler)>(handler),
                           boost::asio::error::already_connected);
            break;

        default:
            invoke_handler(std::forward<decltype(handler)>(handler),
                           boost::asio::error::already_started);
            break;
        }
    }
    result.get();
}

template <typename AcceptHandler>
void acceptor::process_accept(const boost::system::error_code& error,
                              socket_type& socket,
                              AcceptHandler&& handler)
{
    switch (error.value())
    {
    case 0:
        // FIXME: set local_endpoint?
        multiplexer->add(&socket);
        break;

    case boost::asio::error::operation_aborted:
    default:
        break;
    }
    handler(error);
}

inline
acceptor::endpoint_type acceptor::local_endpoint() const
{
    assert(multiplexer);

    return multiplexer->next_layer().local_endpoint();
}

template <typename Handler,
          typename ErrorCode>
void acceptor::invoke_handler(Handler&& handler,
                              ErrorCode error)
{
    assert(error);

    get_io_service().post
        ([handler, error]() mutable
         {
             handler(boost::asio::error::make_error_code(error));
         });
}

} // namespace crux
} // namespace maidsafe

#endif // MAIDSAFE_CRUX_ACCEPTOR_HPP
