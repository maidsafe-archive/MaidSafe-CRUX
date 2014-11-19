///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2014 MaidSafe.net Limited
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
///////////////////////////////////////////////////////////////////////////////

#ifndef MAIDSAFE_CRUX_DETAIL_RECEIVE_INPUT_TYPE_HPP
#define MAIDSAFE_CRUX_DETAIL_RECEIVE_INPUT_TYPE_HPP

#include <maidsafe/crux/detail/buffer.hpp>
#include <maidsafe/crux/detail/header.hpp>

namespace maidsafe { namespace crux { namespace detail {

struct receive_input_type
{
    using read_handler_type
        = std::function<void (const boost::system::error_code&, std::size_t)>;

    header_data_type                         header_data;
    std::vector<boost::asio::mutable_buffer> buffers;
    read_handler_type                        handler;

    template<class MutableBufferSequence>
    receive_input_type( const MutableBufferSequence& payload
                      , read_handler_type&&          handler);
};

}}} // namespace maidsafe::crux::detail

namespace maidsafe { namespace crux { namespace detail {

template<class MutableBufferSequence>
receive_input_type::receive_input_type( const MutableBufferSequence& buffers
                                      , read_handler_type&& handler)
    : handler(std::move(handler))
{
    using boost::asio::mutable_buffer;

    this->buffers.emplace_back(mutable_buffer(&header_data.front()
                                             , header_data.size()));

    for (const auto& buffer : buffers) {
        this->buffers.emplace_back(buffer);
    }
}

}}} // namespace maidsafe::crux::detail

#endif // ifndef MAIDSAFE_CRUX_DETAIL_RECEIVE_INPUT_TYPE_HPP

