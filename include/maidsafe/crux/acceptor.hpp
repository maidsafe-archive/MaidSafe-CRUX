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

    template <typename AcceptHandler>
    void async_accept(socket_type& socket,
                      AcceptHandler&& handler);

    endpoint_type local_endpoint() const;

private:
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

template <typename AcceptHandler>
void acceptor::async_accept(socket_type& socket,
                            AcceptHandler&& handler)
{
    assert(multiplexer);

    multiplexer->async_accept
        (socket,
         [this, &socket, handler]
         (const boost::system::error_code& error)
         {
             this->process_accept(error, socket, handler);
         });
}

template <typename AcceptHandler>
void acceptor::process_accept(const boost::system::error_code& error,
                              socket_type& socket,
                              AcceptHandler&& handler)
{
    switch (error.value())
    {
    case 0:
        {
            // FIXME: set local_endpoint?
            socket.set_multiplexer(multiplexer);
            multiplexer->add(&socket);
            boost::system::error_code success;
            handler(success);
        }
        break;

    case boost::asio::error::operation_aborted:
    default:
        handler(error);
        break;
    }
}

inline
acceptor::endpoint_type acceptor::local_endpoint() const
{
    assert(multiplexer);

    return multiplexer->next_layer().local_endpoint();
}

} // namespace crux
} // namespace maidsafe

#endif // MAIDSAFE_CRUX_ACCEPTOR_HPP
