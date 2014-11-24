///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2014 MaidSafe.net Limited
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
///////////////////////////////////////////////////////////////////////////////

#ifndef MAIDSAFE_CRUX_DETAIL_CONCATENATE_HPP
#define MAIDSAFE_CRUX_DETAIL_CONCATENATE_HPP

namespace maidsafe { namespace crux { namespace detail {

template<typename LeftBuffers, typename RightBuffers>
class concatenated {
    using left_iterator  = typename LeftBuffers::const_iterator;
    using right_iterator = typename RightBuffers::const_iterator;

public:
    using value_type = boost::asio::mutable_buffer;

    class const_iterator {
    public:
        const_iterator& operator++();    // pre-increment
        const_iterator  operator++(int); // post-increment

        const value_type& operator*();
        value_type* operator->();

        bool operator==(const_iterator) const;
        bool operator!=(const_iterator) const;

    private:
        friend concatenated;

        const_iterator( left_iterator  left_begin,  left_iterator  left_end
                      , right_iterator right_begin, right_iterator right_end)
            : left_i(left_begin), left_end(left_end)
            , right_i(right_begin), right_end(right_end)
        {}

    private:
        left_iterator  left_i;
        left_iterator  left_end;

        right_iterator right_i;
        right_iterator right_end;
    };

    template<typename Left, typename Right>
    concatenated(Left&& left, Right&& right)
        : left(std::forward<Left>(left))
        , right(std::forward<Right>(right))
    {}

    const_iterator begin() const;
    const_iterator end()   const;

private:
    LeftBuffers  left;
    RightBuffers right;
};

template<typename Left, typename Right>
typename concatenated<Left, Right>::const_iterator&
concatenated<Left, Right>::const_iterator::operator++() {
    if (left_i != left_end) {
        ++left_i;
    }
    else {
        ++right_i;
    }
    return *this;
}

template<typename Left, typename Right>
typename concatenated<Left, Right>::const_iterator
concatenated<Left, Right>::const_iterator::operator++(int) {
    if (left_i != left_end) {
        return const_iterator(left_i++, left_end, right_i, right_end);
    }
    else {
        return const_iterator(left_i, left_end, right_i++, right_end);
    }
}

template<typename Left, typename Right>
const typename concatenated<Left, Right>::value_type&
concatenated<Left, Right>::const_iterator::operator*() {
    return (left_i!= left_end) ? *left_i
                               : *right_i;
}

template<typename Left, typename Right>
typename concatenated<Left, Right>::value_type*
concatenated<Left, Right>::const_iterator::operator->() {
    return &(this->operator*());
}

template<typename Left, typename Right>
bool concatenated<Left, Right>::const_iterator::operator==(const_iterator other) const {
    return left_i == other.left_i && right_i == other.right_i;
}

template<typename Left, typename Right>
bool concatenated<Left, Right>::const_iterator::operator!=(const_iterator other) const {
    return !this->operator==(other);
}

template<typename Left, typename Right>
typename concatenated<Left, Right>::const_iterator
concatenated<Left, Right>::begin() const {
    return const_iterator( left.begin(), left.end()
                         , right.begin(), right.end());
}

template<typename Left, typename Right>
typename concatenated<Left, Right>::const_iterator
concatenated<Left, Right>::end() const {
    return const_iterator(left.end(), left.end(), right.end(), right.end());
}

template<typename Left, typename Right>
concatenated< typename std::decay<Left>::type
            , typename std::decay<Right>::type>
concatenate(Left&& left, Right&& right) {
    return concatenated< typename std::decay<Left>::type
                       , typename std::decay<Right>::type
                       >( std::forward<Left>(left)
                        , std::forward<Right>(right));
}

}}} // namespace maidsafe::crux::detail


#endif // ifndef MAIDSAFE_CRUX_DETAIL_CONCATENATE_HPP

