///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2014 MaidSafe.net Limited
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
///////////////////////////////////////////////////////////////////////////////

#ifndef MAIDSAFE_CRUX_DETAIL_ENCODER_HPP
#define MAIDSAFE_CRUX_DETAIL_ENCODER_HPP

#include <cstdint>

namespace maidsafe
{
namespace crux
{
namespace detail
{

class encoder
{
public:
    using iterator = std::uint8_t *;

    template <typename RandomAccessIterator>
    encoder(RandomAccessIterator begin, RandomAccessIterator end);

    bool empty() const;
    std::size_t size() const;

    template <typename T>
    void put(T value);

private:
    struct
    {
        iterator begin;
        iterator end;
    } current;
};

} // namespace detail
} // namespace crux
} // namespace maidsafe

#include <cassert>

namespace maidsafe
{
namespace crux
{
namespace detail
{

template <typename RandomAccessIterator>
encoder::encoder(RandomAccessIterator begin,
                 RandomAccessIterator end)
{
    current.begin = reinterpret_cast<iterator>(begin);
    current.end = reinterpret_cast<iterator>(end);
}

inline bool encoder::empty() const
{
    return current.begin == current.end;
}

inline std::size_t encoder::size() const
{
    return current.end - current.begin;
}

template <>
inline void encoder::put(std::uint8_t value)
{
    assert(size() >= sizeof(value));

    *current.begin++ = value;
}

template <>
inline void encoder::put(std::uint16_t value)
{
    assert(size() >= sizeof(value));

    current.begin[0] = (value >> 8) & 0xFF;
    current.begin[1] = value & 0xFF;
    current.begin += sizeof(value);
}

template <>
inline void encoder::put(std::uint32_t value)
{
    assert(size() >= sizeof(value));

    current.begin[0] = (value >> 24) & 0xFF;
    current.begin[1] = (value >> 16) & 0xFF;
    current.begin[2] = (value >> 8) & 0xFF;
    current.begin[3] = value & 0xFF;
    current.begin += sizeof(value);
}

} // namespace detail
} // namespace crux
} // namespace maidsafe

#endif // MAIDSAFE_CRUX_DETAIL_ENCODER_HPP
