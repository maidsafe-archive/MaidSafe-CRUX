///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2014 MaidSafe.net Limited
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
///////////////////////////////////////////////////////////////////////////////

#ifndef MAIDSAFE_CRUX_DETAIL_BUFFER_HPP
#define MAIDSAFE_CRUX_DETAIL_BUFFER_HPP

#include <algorithm>
#include <vector>

namespace maidsafe
{
namespace crux
{
namespace detail
{

using buffer = std::vector<char>;

} // namespace detail
} // namespace crux
} // namespace maidsafe

#include <boost/asio/buffer.hpp>

namespace boost
{
namespace asio
{

inline mutable_buffers_1 buffer(maidsafe::crux::detail::buffer& data)
{
    return mutable_buffers_1(data.empty() ? nullptr : data.data(),
                             data.size() * sizeof(char));
}

inline const_buffers_1 buffer(const maidsafe::crux::detail::buffer& data)
{
    return const_buffers_1(data.empty() ? nullptr : data.data(),
                           data.size() * sizeof(char));
}

} // namespace asio
} // namespace boost

#endif // MAIDSAFE_CRUX_DETAIL_BUFFER_HPP
