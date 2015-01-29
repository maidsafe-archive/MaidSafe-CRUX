///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2014 MaidSafe.net Limited
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
///////////////////////////////////////////////////////////////////////////////

#ifndef MAIDSAFE_CRUX_DETAIL_SEQUENCE_NUMBER_HPP
#define MAIDSAFE_CRUX_DETAIL_SEQUENCE_NUMBER_HPP

#include <type_traits>

namespace maidsafe { namespace crux { namespace detail {

template< typename NumericType
        , NumericType Max = std::numeric_limits<NumericType>::max()
        >
class sequence_number {

    static_assert( std::is_integral<NumericType>::value
                   && std::is_unsigned<NumericType>::value
                 , "NumericType must be an unsigned integral type");

public:
    using value_type = NumericType;
    using difference_type = std::ptrdiff_t;
    static constexpr value_type max_value = Max;

public:
    sequence_number();
    sequence_number(const sequence_number&);
    explicit sequence_number(NumericType);

    sequence_number& operator=(const sequence_number&);
    sequence_number& operator++();    // pre-increment
    sequence_number  operator++(int); // post-increment

    bool operator<(const sequence_number&) const;
    bool operator==(const sequence_number&) const;
    bool operator==(const NumericType&) const;

    difference_type distance(const sequence_number&) const;

    value_type value() const { return n; }

private:
    value_type n;
};

}}} // namespace maidsafe::crux::detail

namespace maidsafe { namespace crux { namespace detail {

template<typename NumericType, NumericType Max>
sequence_number<NumericType, Max>::sequence_number()
    : n(0)
{
}

template<typename NumericType, NumericType Max>
sequence_number<NumericType, Max>::sequence_number(const sequence_number& other)
    : n(other.n)
{
    if (n > max_value)
        throw std::runtime_error("invalid value");
}

template<typename NumericType, NumericType Max>
sequence_number<NumericType, Max>::sequence_number(NumericType n)
    : n(n)
{
    if (n > max_value)
        throw std::runtime_error("invalid value");
}

template<typename NumericType, NumericType Max>
sequence_number<NumericType, Max>&
sequence_number<NumericType, Max>::operator=(const sequence_number& other)
{
    if (this != &other)
    {
        n = other.n;
    }
    return *this;
}

template<typename NumericType, NumericType Max>
sequence_number<NumericType, Max>&
sequence_number<NumericType, Max>::operator++() {
    if (n < max_value) { ++n; } else { n = 0; }
    return *this;
}

template<typename NumericType, NumericType Max>
sequence_number<NumericType, Max>
sequence_number<NumericType, Max>::operator++(int) {
    NumericType old_n = n;
    if (n < max_value) { ++n; } else { n = 0; }
    return sequence_number(old_n);
}

template<typename NumericType, NumericType Max>
bool sequence_number<NumericType, Max>::operator<(const sequence_number& other) const
{
    return ((other.n > n) && (other.n - n <= max_value/2))
        || ((other.n < n) && (n - other.n >  max_value/2));
}

template<typename NumericType, NumericType Max>
bool sequence_number<NumericType, Max>::operator==(const sequence_number& other) const
{
    return n == other.n;
}

template<typename NumericType, NumericType Max>
bool sequence_number<NumericType, Max>::operator==(const NumericType& other) const
{
    return n == other;
}

template<typename NumericType, NumericType Max>
typename sequence_number<NumericType, Max>::difference_type
sequence_number<NumericType, Max>::distance(const sequence_number& other) const
{
    if ((*this < other) && (n <= other.n))
        return other.n - n;
    else
        return other.n + (max_value - n) + 1;
}

}}} // namespace maidsafe::crux::detail

#endif // ifndef MAIDSAFE_CRUX_DETAIL_SEQUENCE_NUMBER_HPP

