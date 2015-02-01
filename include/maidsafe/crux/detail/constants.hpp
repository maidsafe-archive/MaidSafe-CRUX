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

const std::chrono::seconds retransmission_period(3);
const std::chrono::seconds keepalive_timeout(5*retransmission_period);

} // namespace constant
} // namespace detail
} // namespace crux
} // namespace maidsafe

#endif // MAIDSAFE_CRUX_DETAIL_CONSTANTS_HPP

