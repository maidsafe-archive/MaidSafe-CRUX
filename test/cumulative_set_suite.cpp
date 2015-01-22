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

ostream& operator << (ostream& stream, const sequence_number& number)
{
    stream << number.value();
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
    BOOST_REQUIRE_EQUAL(front->first, one);
    BOOST_REQUIRE_EQUAL(front->second, std::uint16_t(0));
}

BOOST_AUTO_TEST_CASE(pop_two)
{
    cumulative_set history;
    sequence_number one(41);
    sequence_number two(42);

    history.insert(one);
    history.insert(two);
    auto front = history.front();
    BOOST_REQUIRE_EQUAL(front->first, two);
    BOOST_REQUIRE_EQUAL(front->second, std::uint16_t(0));
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
    BOOST_REQUIRE_EQUAL(front->first, two);
    BOOST_REQUIRE_EQUAL(front->second, std::uint16_t(0x0001));
    history.insert(three);
    auto front2 = history.front();
    BOOST_REQUIRE_EQUAL(front2->first, four);
    BOOST_REQUIRE_EQUAL(front2->second, std::uint16_t(0));
}

BOOST_AUTO_TEST_CASE(nack_nothing)
{
    cumulative_set history;
    history.insert(sequence_number(1));
    history.insert(sequence_number(2));
    auto front = history.front();
    BOOST_REQUIRE_EQUAL(front->first, 2);
    BOOST_REQUIRE_EQUAL(front->second, std::uint16_t(0x0000));
}

BOOST_AUTO_TEST_CASE(nack_one)
{
    cumulative_set history;
    history.insert(sequence_number(1));
    history.insert(sequence_number(3));
    auto front = history.front();
    BOOST_REQUIRE_EQUAL(front->first, 1);
    BOOST_REQUIRE_EQUAL(front->second, std::uint16_t(0x0001));
}

BOOST_AUTO_TEST_CASE(nack_two)
{
    cumulative_set history;
    history.insert(sequence_number(1));
    history.insert(sequence_number(4));
    auto front = history.front();
    BOOST_REQUIRE_EQUAL(front->first, 1);
    BOOST_REQUIRE_EQUAL(front->second, std::uint16_t(0x0003));
}

BOOST_AUTO_TEST_CASE(nack_three)
{
    cumulative_set history;
    history.insert(sequence_number(1));
    history.insert(sequence_number(5));
    auto front = history.front();
    BOOST_REQUIRE_EQUAL(front->first, 1);
    BOOST_REQUIRE_EQUAL(front->second, std::uint16_t(0x0007));
}

BOOST_AUTO_TEST_CASE(nack_beyond)
{
    cumulative_set history;
    history.insert(sequence_number(1));
    history.insert(sequence_number(99));
    auto front = history.front();
    BOOST_REQUIRE_EQUAL(front->first, 1);
    BOOST_REQUIRE_EQUAL(front->second, std::uint16_t(0xFFFF));
}

BOOST_AUTO_TEST_CASE(nack_every_second)
{
    cumulative_set history;
    history.insert(sequence_number(1));
    history.insert(sequence_number(3));
    history.insert(sequence_number(5));
    history.insert(sequence_number(7));
    history.insert(sequence_number(9));
    history.insert(sequence_number(11));
    history.insert(sequence_number(13));
    history.insert(sequence_number(15));
    auto front = history.front();
    BOOST_REQUIRE_EQUAL(front->first, 1);
    BOOST_REQUIRE_EQUAL(front->second, std::uint16_t(0x1555));
}

BOOST_AUTO_TEST_CASE(nack_every_second_beyond)
{
    cumulative_set history;
    history.insert(sequence_number(1));
    history.insert(sequence_number(3));
    history.insert(sequence_number(5));
    history.insert(sequence_number(7));
    history.insert(sequence_number(9));
    history.insert(sequence_number(11));
    history.insert(sequence_number(13));
    history.insert(sequence_number(15));
    history.insert(sequence_number(17));
    auto front = history.front();
    BOOST_REQUIRE_EQUAL(front->first, 1);
    BOOST_REQUIRE_EQUAL(front->second, std::uint16_t(0x5555));
}

BOOST_AUTO_TEST_SUITE_END()
