///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2014 MaidSafe.net Limited
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
///////////////////////////////////////////////////////////////////////////////

#ifndef MAIDSAFE_CRUX_DETAIL_DECODER_HPP
#define MAIDSAFE_CRUX_DETAIL_DECODER_HPP

#include <cstdint>

namespace maidsafe
{
namespace crux
{
namespace detail
{

class decoder
{
public:
    using const_iterator = const std::uint8_t *;

    template <typename RandomAccessIterator>
    decoder(RandomAccessIterator begin, RandomAccessIterator end);

    bool empty() const;
    std::size_t size() const;

    template <typename T>
    struct return_type { typedef T type; };

    template <typename T>
    typename return_type<T>::type get() const;

private:
    struct
    {
        mutable const_iterator begin;
        const_iterator end;
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
decoder::decoder(RandomAccessIterator begin,
                 RandomAccessIterator end)
{
    current.begin = reinterpret_cast<const_iterator>(begin);
    current.end = reinterpret_cast<const_iterator>(end);
}

inline bool decoder::empty() const
{
    return current.begin == current.end;
}

inline std::size_t decoder::size() const
{
    return current.end - current.begin;
}

template <>
inline decoder::return_type<std::uint8_t>::type decoder::get<std::uint8_t>() const
{
    assert(!empty());
    return *(current.begin++);
}

template <>
inline decoder::return_type<std::uint16_t>::type decoder::get<std::uint16_t>() const
{
    std::uint16_t result;
    assert(reinterpret_cast<std::uintptr_t>(current.begin) % sizeof(result) == 0);
    assert(size() >= sizeof(result));

    result
        = current.begin[0] << 8
        | current.begin[1];

    current.begin += sizeof(result);
    return result;
}

template <>
inline decoder::return_type<std::uint32_t>::type decoder::get<std::uint32_t>() const
{
    std::uint32_t result;
    assert(reinterpret_cast<std::uintptr_t>(current.begin) % sizeof(result) == 0);
    assert(size() >= sizeof(result));

    result
        = current.begin[0] << 24
        | current.begin[1] << 16
        | current.begin[2] << 8
        | current.begin[3];

    current.begin += sizeof(result);
    return result;
}

} // namespace detail
} // namespace crux
} // namespace maidsafe

#endif // MAIDSAFE_CRUX_DETAIL_DECODER_HPP
