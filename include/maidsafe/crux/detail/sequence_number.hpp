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

template<typename NumericType>
class sequence_number {
    static_assert( std::is_integral<NumericType>::value
                 , "NumericType must be an integral type");
    static_assert( std::is_unsigned<NumericType>::value
                 , "NumericType must be an unsigned integral type");

public:
    sequence_number();
    explicit sequence_number(NumericType);

    sequence_number& operator++();    // pre-increment
    sequence_number  operator++(int); // post-increment

    bool operator<(sequence_number) const;
    bool operator==(sequence_number) const;

private:
    NumericType n;
};

}}} // namespace maidsafe::crux::detail

namespace maidsafe { namespace crux { namespace detail {

template<typename NumericType>
sequence_number<NumericType>::sequence_number()
    : n(0) {}

template<typename NumericType>
sequence_number<NumericType>::sequence_number(NumericType n)
    : n(n) {}

template<typename NumericType>
sequence_number<NumericType>& sequence_number<NumericType>::operator++() {
    ++n;
    return *this;
}

template<typename NumericType>
sequence_number<NumericType> sequence_number<NumericType>::operator++(int) {
    return sequence_number(n++);
}

template<typename NumericType>
bool sequence_number<NumericType>::operator<(sequence_number other) const {
    constexpr auto max = std::numeric_limits<NumericType>::max();

    return (other.n > n) && (other.n - n <= max/2) 
        || (other.n < n) && (n - other.n >  max/2);
}

template<typename NumericType>
bool sequence_number<NumericType>::operator==(sequence_number other) const {
    return n == other.n;
}

}}} // namespace maidsafe::crux::detail

#endif // ifndef MAIDSAFE_CRUX_DETAIL_SEQUENCE_NUMBER_HPP

