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
#include <maidsafe/crux/detail/concatenate.hpp>
#include <boost/asio/buffer.hpp>

namespace asio = boost::asio;

// To disable 'unused variable' warnings.
template<class Param> void do_nothing(const Param&) {}

template<std::size_t N>
std::string to_string(const std::array<char, N>& v) {
    return std::string(v.begin(), v.end());
}

BOOST_AUTO_TEST_CASE(concatenate_const_const)
{
    const std::array<char, 10> a10{};
    const std::array<char, 20> a20{};

    auto concatenated = maidsafe::crux::detail::concatenate( asio::buffer(a10)
                                                           , asio::buffer(a20));

    std::size_t size = 0;

    for (const auto& b : concatenated) {
        do_nothing(b);
        ++size;
    }

    BOOST_REQUIRE_EQUAL(2, size);

    BOOST_REQUIRE_EQUAL( a10.size() + a20.size()
                       , asio::buffer_size(concatenated));
}

BOOST_AUTO_TEST_CASE(concatenate_const_mutable)
{
    const std::array<char, 10> a10{};
    std::array<char, 20> a20{};

    auto concatenated = maidsafe::crux::detail::concatenate( asio::buffer(a10)
                                                           , asio::buffer(a20));

    std::size_t size = 0;

    for (const auto& b : concatenated) {
        do_nothing(b);
        ++size;
    }

    BOOST_REQUIRE_EQUAL(2, size);

    BOOST_REQUIRE_EQUAL( a10.size() + a20.size()
                       , asio::buffer_size(concatenated));
}

BOOST_AUTO_TEST_CASE(concatenate_mutable_const)
{
    std::array<char, 10> a10{};
    const std::array<char, 20> a20{};

    auto concatenated = maidsafe::crux::detail::concatenate( asio::buffer(a10)
                                                           , asio::buffer(a20));

    std::size_t size = 0;

    for (const auto& b : concatenated) {
        do_nothing(b);
        ++size;
    }

    BOOST_REQUIRE_EQUAL(2, size);

    BOOST_REQUIRE_EQUAL( a10.size() + a20.size()
                       , asio::buffer_size(concatenated));
}

BOOST_AUTO_TEST_CASE(concatenate_mutable_mutable)
{
    std::array<char, 10> a10{};
    std::array<char, 20> a20{};

    auto concatenated = maidsafe::crux::detail::concatenate( asio::buffer(a10)
                                                           , asio::buffer(a20));

    std::size_t size = 0;

    for (const auto& b : concatenated) {
        do_nothing(b);
        ++size;
    }

    BOOST_REQUIRE_EQUAL(2, size);

    BOOST_REQUIRE_EQUAL( a10.size() + a20.size()
                       , asio::buffer_size(concatenated));
}

BOOST_AUTO_TEST_CASE(concatenate_move)
{
    using maidsafe::crux::detail::concatenate;

    // Check if moving works correctly:
    std::vector<asio::mutable_buffer> v1(10);
    std::vector<asio::mutable_buffer> v2(20);

    concatenate(std::move(v1), v2);

    BOOST_REQUIRE_EQUAL(0, v1.size());
    BOOST_REQUIRE_EQUAL(20, v2.size());
}

BOOST_AUTO_TEST_CASE(concatenate_copy)
{
    using maidsafe::crux::detail::concatenate;

    std::array<char, 10> a10;
    std::array<char, 20> a20;

    auto concatenated = concatenate(asio::buffer(a10), asio::buffer(a20));

    std::size_t size = 0;

    for (const auto& b : concatenated) {
        do_nothing(b);
        ++size;
    }

    BOOST_REQUIRE_EQUAL(2, size);

    BOOST_REQUIRE_EQUAL( a10.size() + a20.size()
                       , asio::buffer_size(concatenated));

    std::string test_data = "012345678910111213141516171819";

    asio::buffer_copy( concatenated
                     , asio::buffer(&test_data[0], test_data.size()));

    BOOST_REQUIRE_EQUAL(to_string(a10), "0123456789");
    BOOST_REQUIRE_EQUAL(to_string(a20), "10111213141516171819");
}

