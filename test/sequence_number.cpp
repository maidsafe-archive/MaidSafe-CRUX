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
#include <maidsafe/crux/detail/sequence_number.hpp>

using maidsafe::crux::detail::sequence_number;

template<class SN> void test_limits()
{
    constexpr auto max = SN::max_value;

    BOOST_REQUIRE(SN(0)       < SN(1));
    BOOST_REQUIRE(SN(0)       < SN(max/2));
    BOOST_REQUIRE(SN(max)     < SN(max/2));
    BOOST_REQUIRE(SN(max/2+1) < SN(0));
    BOOST_REQUIRE(SN(max/2+1) < SN(max));
    BOOST_REQUIRE(SN(max)     < SN(0));
}

BOOST_AUTO_TEST_CASE(sequence_number_uint8)
{
    test_limits<sequence_number<uint8_t>>();
}

BOOST_AUTO_TEST_CASE(sequence_number_uint16)
{
    test_limits<sequence_number<uint16_t>>();
}

BOOST_AUTO_TEST_CASE(sequence_number_uint32)
{
    test_limits<sequence_number<uint32_t>>();
}

BOOST_AUTO_TEST_CASE(sequence_number_uint32_custom_max)
{
    using T = uint32_t;
    constexpr T max = std::numeric_limits<T>::max();

    test_limits<sequence_number<T, max/2>>();
    test_limits<sequence_number<T, max/4>>();
    test_limits<sequence_number<T, max/8>>();
    test_limits<sequence_number<T, max/16>>();
}

BOOST_AUTO_TEST_CASE(sequence_number_pre_increment)
{
    using T = uint8_t;
    constexpr T max = std::numeric_limits<T>::max()/2;
    sequence_number<T, max> s;
    uint32_t counter = 0;
    while ((++s).value()) { ++counter; }
    BOOST_REQUIRE_EQUAL(counter, max);
}

BOOST_AUTO_TEST_CASE(sequence_number_post_increment)
{
    using T = uint8_t;
    constexpr T max = std::numeric_limits<T>::max()/2;
    sequence_number<T, max> s;
    s++;
    uint32_t counter = 0;
    while ((s++).value()) { ++counter; }
    BOOST_REQUIRE_EQUAL(counter, max);
}

