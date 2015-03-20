///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2014 MaidSafe.net Limited
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
///////////////////////////////////////////////////////////////////////////////

#ifndef MAIDSAFE_CRUX_DETAIL_HEADER_HPP
#define MAIDSAFE_CRUX_DETAIL_HEADER_HPP

#include <boost/optional.hpp>
#include <maidsafe/crux/detail/decoder.hpp>
#include <maidsafe/crux/detail/encoder.hpp>
#include <maidsafe/crux/detail/header_constants.hpp>
#include <maidsafe/crux/detail/sequence_number.hpp>

namespace maidsafe
{
namespace crux
{
namespace detail
{
namespace header
{

using sequence_type = sequence_number<std::uint32_t>;

struct empty {

    empty() {}

    empty(std::uint16_t type, detail::decoder&)
    {
        static_cast<void>(type);
        assert((type & header::constant::mask_type) == header::constant::type_empty);
    }

    void encode(detail::encoder& encoder) const {
        encoder.put<std::uint16_t>(header::constant::type_empty);
    }
};

struct handshake {
    std::size_t                    retransmission_count;
    std::uint16_t                  version;
    sequence_type                  initial_sequence_number;
    boost::optional<sequence_type> ack;

    handshake( std::size_t                    retransmission_count
             , sequence_type                  initial_sequence_number
             , boost::optional<sequence_type> ack)
        : retransmission_count(retransmission_count)
        , version(header::constant::version)
        , initial_sequence_number(initial_sequence_number)
        , ack(ack)
    {}

    handshake(std::uint16_t type, detail::decoder& decoder)
        : retransmission_count(type & 3)
        , version(decoder.get<std::uint16_t>())
        , initial_sequence_number(decoder.get<std::uint32_t>())
    {
        assert((type & header::constant::mask_type) == header::constant::type_handshake);

        if (type & header::constant::mask_ack) {
            ack = sequence_type(decoder.get<std::uint32_t>());
        }
    }

    void encode(detail::encoder& encoder) const {
        encoder.put<std::uint16_t>(
            header::constant::type_handshake
            | static_cast<std::uint16_t>(std::min<std::size_t>(3, retransmission_count))
            | (ack ? header::constant::ack_type_cumulative : header::constant::ack_type_none));
        encoder.put<std::uint16_t>(version);
        encoder.put<std::uint32_t>(initial_sequence_number.value());
        encoder.put<std::uint32_t>(ack ? ack->value() : 0);
    }
};

struct keepalive {
    std::size_t                    retransmission_count;
    std::uint16_t                  dummy;
    sequence_type                  sequence_number;
    boost::optional<sequence_type> ack;

    keepalive( std::size_t                    retransmission_count
             , sequence_type                  sequence_number
             , boost::optional<sequence_type> ack)
        : retransmission_count(retransmission_count)
        , dummy(0)
        , sequence_number(sequence_number)
        , ack(ack)
    {}

    keepalive(std::uint16_t type, detail::decoder& decoder)
        : retransmission_count(3 & type)
        , dummy(decoder.get<std::uint16_t>())
        , sequence_number(decoder.get<std::uint32_t>())
    {
        assert((type & header::constant::mask_type) == header::constant::type_keepalive);

        if (type & header::constant::mask_ack) {
            ack = sequence_type(decoder.get<std::uint32_t>());
        }
    }

    void encode(detail::encoder& encoder) const {
        encoder.put<std::uint16_t>(
            header::constant::type_keepalive
            | static_cast<std::uint16_t>(std::min<std::size_t>(3, retransmission_count))
            | (ack ? header::constant::ack_type_cumulative : header::constant::ack_type_none));
        encoder.put<std::uint16_t>(0);
        encoder.put<std::uint32_t>(sequence_number.value());
        encoder.put<std::uint32_t>(ack ? ack->value() : 0);
    }
};

struct data {
    std::uint16_t                  retransmission_count;
    std::uint16_t                  dummy;
    sequence_type                  sequence_number;
    boost::optional<sequence_type> ack;

    data( std::uint16_t                  retransmission_count
        , sequence_type                  sequence_number
        , boost::optional<sequence_type> ack)
            : retransmission_count(retransmission_count)
            , dummy(0)
            , sequence_number(sequence_number)
            , ack(ack)
    { }

    data(std::uint16_t type, detail::decoder& decoder)
        : retransmission_count(type & 3)
        , dummy(decoder.get<std::uint16_t>())
        , sequence_number(decoder.get<std::uint32_t>())
    {
        assert((type & header::constant::mask_type) == header::constant::type_data);

        if (type & header::constant::mask_ack)
        {
            ack = sequence_type(decoder.get<std::uint32_t>());
        }
    }

    void encode(detail::encoder& encoder) const {
        encoder.put<std::uint16_t>(
            header::constant::type_data
            | static_cast<std::uint16_t>(std::min<std::size_t>(3, retransmission_count))
            | (ack ? header::constant::ack_type_cumulative : header::constant::ack_type_none));
        encoder.put<std::uint16_t>(0);
        encoder.put<std::uint32_t>(sequence_number.value());
        encoder.put<std::uint32_t>(ack ? ack->value() : 0);
    }
};

} // namespace header
} // namespace detail
} // namespace crux
} // namespace maidsafe

#endif // MAIDSAFE_CRUX_DETAIL_HEADER_HPP
