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

namespace maidsafe { namespace crux { namespace detail {

template< typename NumericType
        , NumericType Max = std::numeric_limits<NumericType>::max()
        >
class sequence_number {

    static_assert( std::is_integral<NumericType>::value
                   && std::is_unsigned<NumericType>::value
                 , "NumericType must be an unsigned integral type");

public:
    static constexpr NumericType max_value = Max;

public:
    sequence_number();
    explicit sequence_number(NumericType);

    sequence_number& operator++();    // pre-increment
    sequence_number  operator++(int); // post-increment

    bool operator<(sequence_number) const;
    bool operator==(sequence_number) const;

    NumericType value() const { return n; }

private:
    NumericType n;
};

}}} // namespace maidsafe::crux::detail

namespace maidsafe { namespace crux { namespace detail {

template<typename NumericType, NumericType Max>
sequence_number<NumericType, Max>::sequence_number()
    : n(0) {}

template<typename NumericType, NumericType Max>
sequence_number<NumericType, Max>::sequence_number(NumericType n)
    : n(n)
{
    if (n > max_value)
        throw std::runtime_error("invalid value");
}

template<typename NumericType, NumericType Max>
sequence_number<NumericType, Max>&
sequence_number<NumericType, Max>::operator++() {
    ++n;
    if (n > Max) { n %= Max + 1; }
    return *this;
}

template<typename NumericType, NumericType Max>
sequence_number<NumericType, Max>
sequence_number<NumericType, Max>::operator++(int) {
    NumericType old_n = n;
    ++n;
    if (n > Max) { n %= Max + 1; }
    return sequence_number(old_n);
}

template<typename NumericType, NumericType Max>
bool sequence_number<NumericType, Max>::operator<(sequence_number other) const {

    return ((other.n > n) && (other.n - n <= Max/2))
        || ((other.n < n) && (n - other.n >  Max/2));
}

template<typename NumericType, NumericType Max>
bool sequence_number<NumericType, Max>::operator==(sequence_number other) const {
    return n == other.n;
}

}}} // namespace maidsafe::crux::detail

#endif // ifndef MAIDSAFE_CRUX_DETAIL_SEQUENCE_NUMBER_HPP

