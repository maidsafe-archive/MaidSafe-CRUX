///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2014 MaidSafe.net Limited
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
///////////////////////////////////////////////////////////////////////////////

#ifndef MAIDSAFE_CRUX_DETAIL_HEADER_CONSTANTS_HPP
#define MAIDSAFE_CRUX_DETAIL_HEADER_CONSTANTS_HPP

#include <cstdint>
#include <array>

namespace maidsafe
{
namespace crux
{
namespace detail
{
namespace header
{
namespace constant
{

const std::size_t version = 0;

const std::size_t size =
    sizeof(std::uint16_t) // type
    + sizeof(std::uint16_t) // ack-field
    + sizeof(std::uint32_t) // sequence number
    + sizeof(std::uint32_t); // ack sequence number

const std::uint16_t mask_type = 0XF800;
const std::uint16_t mask_retransmission = 0x0003;
const std::uint16_t mask_ack = 0x000C;

const std::uint16_t type_data = 0xC000;
const std::uint16_t type_handshake = 0xC800;
const std::uint16_t type_shutdown = 0xD000;
const std::uint16_t type_keepalive = 0xD800;

const std::uint16_t ack_type_none = 0x0000;
const std::uint16_t ack_type_cumulative = 0x0004;

} // namespace constant

using data_type = std::array<std::uint8_t, header::constant::size>;

} // namespace header
} // namespace detail
} // namespace crux
} // namespace maidsafe

#endif // MAIDSAFE_CRUX_DETAIL_HEADER_CONSTANTS_HPP
