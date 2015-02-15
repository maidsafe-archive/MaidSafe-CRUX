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
    using composite_type = value_type;

    bool empty() const;

    boost::optional<composite_type> front();

    void insert(const value_type&);

private:
    void prune();

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
    return *cumulative;
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

} // namespace detail
} // namespace crux
} // namespace maidsafe

#endif // MAIDSAFE_CRUX_DETAIL_CUMULATIVE_SET_HPP
