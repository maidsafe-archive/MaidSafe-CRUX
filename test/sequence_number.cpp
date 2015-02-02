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
#include <maidsafe/crux/detail/sequence_number.hpp>

using maidsafe::crux::detail::sequence_number;

template<class SN> void test_limits()
{
    constexpr auto max = SN::max_value;
    constexpr auto half = max/2;

    BOOST_REQUIRE(SN(0)       < SN(1));
    BOOST_REQUIRE(SN(0)       < SN(half-1));
    BOOST_REQUIRE(SN(0)       < SN(half));
    BOOST_REQUIRE(SN(0)       > SN(half+1));
    BOOST_REQUIRE(SN(0)       > SN(max-1));
    BOOST_REQUIRE(SN(0)       > SN(max));
    BOOST_REQUIRE(SN(half-1)  < SN(half));
    BOOST_REQUIRE(SN(half-1)  < SN(half+1));
    BOOST_REQUIRE(SN(half-1)  > SN(max-1));
    BOOST_REQUIRE(SN(half-1)  > SN(max));
    BOOST_REQUIRE(SN(half)    < SN(half+1));
    BOOST_REQUIRE(SN(half)    < SN(max-1));
    BOOST_REQUIRE(SN(half)    > SN(max));
    BOOST_REQUIRE(SN(half+1)  < SN(max-1));
    BOOST_REQUIRE(SN(half+1)  < SN(max));
}

BOOST_AUTO_TEST_SUITE(sequence_suite)

BOOST_AUTO_TEST_CASE(sequence_number_uint8)
{
    test_limits<sequence_number<std::uint8_t>>();
}

BOOST_AUTO_TEST_CASE(sequence_number_uint16)
{
    test_limits<sequence_number<std::uint16_t>>();
}

BOOST_AUTO_TEST_CASE(sequence_number_uint32)
{
    test_limits<sequence_number<std::uint32_t>>();
}

BOOST_AUTO_TEST_CASE(sequence_number_uint32_custom_max)
{
    using T = std::uint32_t;
    constexpr T max = std::numeric_limits<T>::max();

    test_limits<sequence_number<T, max/2>>();
    test_limits<sequence_number<T, max/4>>();
    test_limits<sequence_number<T, max/8>>();
    test_limits<sequence_number<T, max/16>>();
}

BOOST_AUTO_TEST_CASE(sequence_number_pre_increment)
{
    using T = std::uint8_t;
    constexpr T max = std::numeric_limits<T>::max()/2;
    sequence_number<T, max> s;
    std::uint32_t counter = 0;
    while ((++s).value()) { ++counter; }
    BOOST_REQUIRE_EQUAL(counter, max);
}

BOOST_AUTO_TEST_CASE(sequence_number_post_increment)
{
    using T = std::uint8_t;
    constexpr T max = std::numeric_limits<T>::max()/2;
    sequence_number<T, max> s;
    s++;
    std::uint32_t counter = 0;
    while ((s++).value()) { ++counter; }
    BOOST_REQUIRE_EQUAL(counter, max);
}

BOOST_AUTO_TEST_CASE(distances)
{
    using T = std::uint32_t;
    constexpr T max = sequence_number<T>::max_value;
    sequence_number<T> zero(0);
    sequence_number<T> one(1);
    sequence_number<T> half(max/2);
    sequence_number<T> second_largest(max-1);
    sequence_number<T> largest(max);

    BOOST_CHECK_EQUAL(zero.distance(one), 1);
    BOOST_CHECK_EQUAL(one.distance(zero), -1);
    BOOST_CHECK_EQUAL(second_largest.distance(largest), 1);
    BOOST_CHECK_EQUAL(largest.distance(second_largest), -1);
    BOOST_CHECK_EQUAL(largest.distance(zero), 1);
    BOOST_CHECK_EQUAL(zero.distance(largest), -1);
    BOOST_CHECK_EQUAL(zero.distance(half), half.value());
    BOOST_CHECK_EQUAL(half.distance(zero), -half.value());
    BOOST_CHECK_EQUAL(one.distance(half), half.value() - 1);
    BOOST_CHECK_EQUAL(half.distance(one), -(half.value() - 1));
    BOOST_CHECK_EQUAL(largest.distance(half), -(half.value() + 1));
    BOOST_CHECK_EQUAL(half.distance(largest), half.value() + 1);
    BOOST_CHECK_EQUAL(second_largest.distance(half), -half.value());
    BOOST_CHECK_EQUAL(half.distance(second_largest), half.value());
    BOOST_CHECK_EQUAL(largest.distance(one), 2);
    BOOST_CHECK_EQUAL(one.distance(largest), -2);
    BOOST_CHECK_EQUAL(second_largest.distance(one), 3);
    BOOST_CHECK_EQUAL(one.distance(second_largest), -3);
}

BOOST_AUTO_TEST_CASE(short_distances)
{
    using T = std::uint16_t;
    constexpr T max = sequence_number<T>::max_value;
    sequence_number<T> zero(0);
    sequence_number<T> one(1);
    sequence_number<T> half(max/2);
    sequence_number<T> second_largest(max-1);
    sequence_number<T> largest(max);

    BOOST_CHECK_EQUAL(zero.distance(one), 1);
    BOOST_CHECK_EQUAL(one.distance(zero), -1);
    BOOST_CHECK_EQUAL(second_largest.distance(largest), 1);
    BOOST_CHECK_EQUAL(largest.distance(second_largest), -1);
    BOOST_CHECK_EQUAL(largest.distance(zero), 1);
    BOOST_CHECK_EQUAL(zero.distance(largest), -1);
    BOOST_CHECK_EQUAL(zero.distance(half), half.value());
    BOOST_CHECK_EQUAL(half.distance(zero), -half.value());
    BOOST_CHECK_EQUAL(one.distance(half), half.value() - 1);
    BOOST_CHECK_EQUAL(half.distance(one), -(half.value() - 1));
    BOOST_CHECK_EQUAL(largest.distance(half), -(half.value() + 1));
    BOOST_CHECK_EQUAL(half.distance(largest), half.value() + 1);
    BOOST_CHECK_EQUAL(second_largest.distance(half), -half.value());
    BOOST_CHECK_EQUAL(half.distance(second_largest), half.value());
    BOOST_CHECK_EQUAL(largest.distance(one), 2);
    BOOST_CHECK_EQUAL(one.distance(largest), -2);
    BOOST_CHECK_EQUAL(second_largest.distance(one), 3);
    BOOST_CHECK_EQUAL(one.distance(second_largest), -3);
}

BOOST_AUTO_TEST_CASE(custom_short_distances)
{
    using T = std::uint16_t;
    constexpr T max = std::numeric_limits<T>::max() / 2;
    sequence_number<T, max> zero(0);
    sequence_number<T, max> one(1);
    sequence_number<T, max> half(max/2);
    sequence_number<T, max> second_largest(max-1);
    sequence_number<T, max> largest(max);

    BOOST_CHECK_EQUAL(zero.distance(one), 1);
    BOOST_CHECK_EQUAL(one.distance(zero), -1);
    BOOST_CHECK_EQUAL(second_largest.distance(largest), 1);
    BOOST_CHECK_EQUAL(largest.distance(second_largest), -1);
    BOOST_CHECK_EQUAL(largest.distance(zero), 1);
    BOOST_CHECK_EQUAL(zero.distance(largest), -1);
    BOOST_CHECK_EQUAL(zero.distance(half), half.value());
    BOOST_CHECK_EQUAL(half.distance(zero), -half.value());
    BOOST_CHECK_EQUAL(one.distance(half), half.value() - 1);
    BOOST_CHECK_EQUAL(half.distance(one), -(half.value() - 1));
    BOOST_CHECK_EQUAL(largest.distance(half), -(half.value() + 1));
    BOOST_CHECK_EQUAL(half.distance(largest), half.value() + 1);
    BOOST_CHECK_EQUAL(second_largest.distance(half), -half.value());
    BOOST_CHECK_EQUAL(half.distance(second_largest), half.value());
    BOOST_CHECK_EQUAL(largest.distance(one), 2);
    BOOST_CHECK_EQUAL(one.distance(largest), -2);
    BOOST_CHECK_EQUAL(second_largest.distance(one), 3);
    BOOST_CHECK_EQUAL(one.distance(second_largest), -3);
}

BOOST_AUTO_TEST_SUITE_END()
