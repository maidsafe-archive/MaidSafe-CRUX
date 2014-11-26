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

template<class NumericType> void test_limits()
{
    using SN = maidsafe::crux::detail::sequence_number<NumericType>;

    constexpr auto max = std::numeric_limits<NumericType>::max();

    BOOST_REQUIRE(SN(0)       < SN(1));
    BOOST_REQUIRE(SN(0)       < SN(max/2));
    BOOST_REQUIRE(SN(max)     < SN(max/2));
    BOOST_REQUIRE(SN(max/2+1) < SN(0));
    BOOST_REQUIRE(SN(max/2+1) < SN(max));
    BOOST_REQUIRE(SN(max)     < SN(0));
}

BOOST_AUTO_TEST_CASE(sequence_number_uint8)
{
    test_limits<uint8_t>();
}

BOOST_AUTO_TEST_CASE(sequence_number_uint16)
{
    test_limits<uint16_t>();
}

BOOST_AUTO_TEST_CASE(sequence_number_uint32)
{
    test_limits<uint32_t>();
}

