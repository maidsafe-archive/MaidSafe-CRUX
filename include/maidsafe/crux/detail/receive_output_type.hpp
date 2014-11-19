///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2014 MaidSafe.net Limited
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
///////////////////////////////////////////////////////////////////////////////

#ifndef MAIDSAFE_CRUX_DETAIL_RECEIVE_OUTPUT_TYPE_HPP
#define MAIDSAFE_CRUX_DETAIL_RECEIVE_OUTPUT_TYPE_HPP

#include <maidsafe/crux/detail/buffer.hpp>
#include <maidsafe/crux/detail/header.hpp>

namespace maidsafe { namespace crux { namespace detail {

struct receive_output_type
{
    boost::system::error_code          error;
    std::size_t                        size;
    std::shared_ptr<std::vector<char>> data;
};

}}} // namespace maidsafe::crux::detail

#endif // ifndef MAIDSAFE_CRUX_DETAIL_RECEIVE_OUTPUT_TYPE_HPP

