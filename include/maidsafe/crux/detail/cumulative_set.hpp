///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2014 MaidSafe.net Limited
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
///////////////////////////////////////////////////////////////////////////////

#ifndef MAIDSAFE_CRUX_DETAIL_CUMULATIVE_SET_HPP
#define MAIDSAFE_CRUX_DETAIL_CUMULATIVE_SET_HPP

#include <set>
#include <utility>
#include <type_traits>
#include <boost/optional.hpp>

namespace maidsafe
{
namespace crux
{
namespace detail
{

template <typename SequenceType,
          typename FieldType>
class cumulative_set
{
    static_assert(std::is_integral<FieldType>::value && std::is_unsigned<FieldType>::value,
                  "Field type must be an unsigned integral");

    using container_type = std::set<SequenceType>;

public:
    using value_type = typename container_type::value_type;
    using field_type = FieldType;
    using composite_type = std::pair<value_type, field_type>;

    bool empty() const;

    boost::optional<composite_type> front();

    void insert(const value_type&);

private:
    void prune();
    field_type calculate_nack_field(typename container_type::iterator);

private:
    container_type container;
};

} // namespace detail
} // namespace crux
} // namespace maidsafe

#include <algorithm>

namespace maidsafe
{
namespace crux
{
namespace detail
{

template <typename SequenceType, typename FieldType>
bool cumulative_set<SequenceType, FieldType>::empty() const
{
    return container.empty();
}

template <typename SequenceType, typename FieldType>
boost::optional<typename cumulative_set<SequenceType, FieldType>::composite_type>
cumulative_set<SequenceType, FieldType>::front()
{
    if (container.empty())
        return boost::none;

    auto cumulative = container.begin();
    return std::make_pair(*cumulative, calculate_nack_field(cumulative));
}

template <typename SequenceType, typename FieldType>
void cumulative_set<SequenceType, FieldType>::insert(const value_type& item)
{
    container.insert(item);
    prune();
    assert(!container.empty());
}

template <typename SequenceType, typename FieldType>
void cumulative_set<SequenceType, FieldType>::prune()
{
    // Skip contiguous sequence numbers to find most recent cumulative entry.

    // This algorithm works almost like std::adjacent_find but we always want
    // the last tie instead of the first.
    auto current = container.begin();
    if (current != container.end()) {
        auto next = current;
        ++next;
        for (; next != container.end(); ++next, ++current)
        {
            if (next == container.end())
                break;
            if (current->distance(*next) > 1)
                return;
        }
        // Prune all entries before the cumulative value
        container.erase(container.begin(), current);
    }
}

template <typename SequenceType, typename FieldType>
typename cumulative_set<SequenceType, FieldType>::field_type
cumulative_set<SequenceType, FieldType>::calculate_nack_field(typename container_type::iterator origin)
{
    // Calculate negative acknowledgements from cumulative entry
    field_type result = 0;
    const int field_size = sizeof(field_type) * 8;
    auto where = origin;
    ++where;
    auto offset = 0;
    auto delta = 0;
    while (where != container.end())
    {
        auto diff = origin->distance(*where);
        // Negate from offset to diff (unless larger than field_size)
        auto delta = (diff > field_size)
            ? field_size - offset
            : diff - offset - 1;
        result |= ((1 << delta) - 1) << offset;
        offset = diff;
        ++where;
    }
    return result;
}

} // namespace detail
} // namespace crux
} // namespace maidsafe

#endif // MAIDSAFE_CRUX_DETAIL_CUMULATIVE_SET_HPP
