///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2014 MaidSafe.net Limited
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
///////////////////////////////////////////////////////////////////////////////

#ifndef MAIDSAFE_CRUX_DETAIL_CONSTANTS_HPP
#define MAIDSAFE_CRUX_DETAIL_CONSTANTS_HPP

#include <chrono>

namespace maidsafe
{
namespace crux
{
namespace detail
{
namespace constant
{

// According to RFC 5404, section 3.1.2
//  "When implementing [a retransmission] scheme, applications need to choose a
//   sensible initial value for the RTT. This value SHOULD generally be as
//   conservative as possible for the given application.  TCP uses an initial
//   value of 3 seconds [RFC2988], which is also RECOMMENDED as an initial value
//   for UDP applications."
const std::chrono::seconds initial_roundtrip_time(3);

const std::chrono::seconds keepalive_timeout(5*initial_roundtrip_time);

} // namespace constant
} // namespace detail
} // namespace crux
} // namespace maidsafe

#endif // MAIDSAFE_CRUX_DETAIL_CONSTANTS_HPP
