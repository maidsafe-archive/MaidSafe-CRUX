///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2014 MaidSafe.net Limited
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
///////////////////////////////////////////////////////////////////////////////

#include <boost/test/unit_test.hpp>
#include <cstdint>
#include <utility>
#include <maidsafe/crux/detail/cumulative_set.hpp>
#include <maidsafe/crux/detail/sequence_number.hpp>

using sequence_number = maidsafe::crux::detail::sequence_number<std::uint32_t>;
using cumulative_set = maidsafe::crux::detail::cumulative_set<sequence_number, std::uint16_t>;

namespace std
{

ostream& operator << (ostream& stream, const boost::optional<sequence_number>& number)
{
    if (number)
        stream << number->value();
    else
        stream << "no value";
    return stream;
}

} // namespace std

BOOST_AUTO_TEST_SUITE(cumulative_set_suite)

BOOST_AUTO_TEST_CASE(empty)
{
    cumulative_set history;

    BOOST_REQUIRE_EQUAL(history.empty(), true);
}

BOOST_AUTO_TEST_CASE(pop_none)
{
    cumulative_set history;

    BOOST_REQUIRE_EQUAL(history.front(), boost::none);
}

BOOST_AUTO_TEST_CASE(pop_one)
{
    cumulative_set history;
    sequence_number one(41);

    history.insert(one);
    BOOST_REQUIRE_EQUAL(history.empty(), false);
    auto front = history.front();
    BOOST_REQUIRE_EQUAL(front, one);
}

BOOST_AUTO_TEST_CASE(pop_two)
{
    cumulative_set history;
    sequence_number one(41);
    sequence_number two(42);

    history.insert(one);
    history.insert(two);
    auto front = history.front();
    BOOST_REQUIRE_EQUAL(front, two);
    BOOST_REQUIRE_EQUAL(history.empty(), false); // Value 'two' remains
}

BOOST_AUTO_TEST_CASE(out_of_sequence)
{
    cumulative_set history;
    sequence_number one(41);
    sequence_number two(42);
    sequence_number three(43);
    sequence_number four(44);

    history.insert(one);
    history.insert(two);
    history.insert(four);
    auto front = history.front();
    BOOST_REQUIRE_EQUAL(front, two);
    history.insert(three);
    auto front2 = history.front();
    BOOST_REQUIRE_EQUAL(front2, four);
}

BOOST_AUTO_TEST_SUITE_END()
