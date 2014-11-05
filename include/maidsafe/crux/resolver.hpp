///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2014 MaidSafe.net Limited
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
///////////////////////////////////////////////////////////////////////////////

#ifndef MAIDSAFE_CRUX_RESOLVER_HPP
#define MAIDSAFE_CRUX_RESOLVER_HPP

#include <boost/asio/ip/udp.hpp> // ip::udp::resolver
#include <maidsafe/crux/endpoint.hpp>

namespace maidsafe
{
namespace crux
{

class resolver : public boost::asio::ip::udp::resolver
{
    using super = boost::asio::ip::udp::resolver;

public:
    using endpoint_type = crux::endpoint;

    resolver(boost::asio::io_service& io)
        : super(io)
    {}
};

} // namespace crux
} // namespace maidsafe

#endif // MAIDSAFE_CRUX_RESOLVER_HPP
