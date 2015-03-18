///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2014 MaidSafe.net Limited
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
///////////////////////////////////////////////////////////////////////////////

#ifndef MAIDSAFE_CRUX_ENDPOINT_HPP
#define MAIDSAFE_CRUX_ENDPOINT_HPP

#include <maidsafe/crux/detail/config.hpp>

#include <boost/asio/ip/udp.hpp>

namespace maidsafe
{
namespace crux
{

using endpoint = boost::asio::ip::udp::endpoint;

} // namespace crux
} // namespace maidsafe

#endif // MAIDSAFE_CRUX_ENDPOINT_HPP
