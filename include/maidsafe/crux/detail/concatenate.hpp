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

    using left_value_type  = typename LeftBuffers::value_type;
    using right_value_type = typename RightBuffers::value_type;

public:
    using value_type
            = typename std::conditional< std::is_convertible< right_value_type
                                                            , left_value_type
                                                            >::value
                                       , left_value_type
                                       , right_value_type
                                       >::type;

    class const_iterator {
    public:
        const_iterator& operator++();    // pre-increment
        const_iterator  operator++(int); // post-increment

        value_type operator*();
        const value_type& operator*() const;
        const value_type* operator->() const;

        bool operator==(const_iterator) const;
        bool operator!=(const_iterator) const;

    private:
        friend concatenated;

        const_iterator( left_iterator  left_begin,  left_iterator  left_end
                      , right_iterator right_begin, right_iterator right_end)
            : left_current(left_begin), left_end(left_end)
            , right_current(right_begin), right_end(right_end)
        {}

    private:
        left_iterator  left_current;
        left_iterator  left_end;

        right_iterator right_current;
        right_iterator right_end;
    };

    template<typename Left, typename Right>
    concatenated(Left&& left, Right&& right)
        : left(std::forward<Left>(left))
        , right(std::forward<Right>(right))
    {
    }

    const_iterator begin() const;
    const_iterator end()   const;

private:
    LeftBuffers  left;
    RightBuffers right;
};

template<typename Left, typename Right>
typename concatenated<Left, Right>::const_iterator&
concatenated<Left, Right>::const_iterator::operator++() {
    if (left_current != left_end) {
        ++left_current;
    }
    else {
        ++right_current;
    }
    return *this;
}

template<typename Left, typename Right>
typename concatenated<Left, Right>::const_iterator
concatenated<Left, Right>::const_iterator::operator++(int) {
    if (left_current != left_end) {
        return const_iterator(left_current++, left_end, right_current, right_end);
    }
    else {
        return const_iterator(left_current, left_end, right_current++, right_end);
    }
}

template<typename Left, typename Right>
typename concatenated<Left, Right>::value_type
concatenated<Left, Right>::const_iterator::operator*() {
    return (left_current!= left_end) ? *left_current
                                     : *right_current;
}

template<typename Left, typename Right>
const typename concatenated<Left, Right>::value_type&
concatenated<Left, Right>::const_iterator::operator*() const {
    static_assert(std::is_same<left_value_type, right_value_type>::value
                 , "Both buffers must be of same type");
    return (left_current!= left_end) ? *left_current
                                     : *right_current;
}

template<typename Left, typename Right>
const typename concatenated<Left, Right>::value_type*
concatenated<Left, Right>::const_iterator::operator->() const {
    static_assert(std::is_same<left_value_type, right_value_type>::value
                 , "Both buffers must be of same type");
    return &(this->operator*());
}

template<typename Left, typename Right>
bool concatenated<Left, Right>::const_iterator::operator==(const_iterator other) const {
    return left_current == other.left_current
        && right_current == other.right_current;
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

