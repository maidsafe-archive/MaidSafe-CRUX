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
#include <maidsafe/crux/detail/timer.hpp>

namespace asio = boost::asio;
using clock_type    = std::chrono::steady_clock;
using duration_type = clock_type::duration;
using milliseconds  = std::chrono::milliseconds;

////////////////////////////////////////////////////////////////////////////////
namespace std { namespace chrono {

template<class Rep, class Period>
static std::ostream& operator<<
    ( std::ostream& os
    , const std::chrono::duration<Rep, Period>& duration)
{
    using namespace std::chrono;
    return os << duration_cast<milliseconds>(duration).count() << "ms";
}

}} // namespace std::chrono

////////////////////////////////////////////////////////////////////////////////
template<class Rep, class Period>
duration_type abs(const std::chrono::duration<Rep, Period>& duration) {
    if (duration.count() < 0) {
        return duration * (-1);
    }
    return duration;
}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE(timer_start_stop)
{
    using namespace maidsafe;
    using clock = std::chrono::steady_clock;

    asio::io_service ios;

    crux::detail::timer timer(ios);

    auto period_duration = milliseconds(100);
    const unsigned int required_tick_count = 3;
    unsigned int tick_count = 0;

    auto start_time = clock::now();
    auto end_time   = start_time;

    auto timer_handler = [&]() {
        if (++tick_count == required_tick_count) {
            end_time = clock::now();
            timer.stop();
            return;
        }
        timer.start();
    };

    timer.set_period(period_duration);
    timer.set_handler(timer_handler);
    timer.start();

    ios.run();

    auto test_duration = end_time - start_time;
    auto required_duration = period_duration * required_tick_count;

    BOOST_REQUIRE_EQUAL(tick_count, required_tick_count);

    BOOST_REQUIRE_LE(abs(test_duration - required_duration), milliseconds(10));
}

BOOST_AUTO_TEST_CASE(timer_start_restart)
{
    using namespace maidsafe;
    using clock = std::chrono::steady_clock;

    asio::io_service ios;

    crux::detail::timer timer(ios);

    auto period_duration = milliseconds(100);
    const unsigned int stop1_tick_count = 3;
    const unsigned int stop2_tick_count = 3;
    unsigned int tick_count = 0;

    auto start_time = clock::now();
    auto end_time   = start_time;

    auto timer_handler = [&]() {
        ++tick_count;

        if (tick_count == stop1_tick_count) {
            timer.stop();
            timer.start();
            return;
        }

        if (tick_count == stop1_tick_count + stop2_tick_count) {
            end_time = clock::now();
            timer.stop();
            return;
        }

        timer.start();
    };

    timer.set_period(period_duration);
    timer.set_handler(timer_handler);
    timer.start();

    ios.run();

    auto test_duration = end_time - start_time;
    auto required_duration = period_duration * (stop1_tick_count + stop2_tick_count);

    BOOST_REQUIRE_EQUAL(tick_count, stop1_tick_count + stop2_tick_count);
    BOOST_REQUIRE_LE(abs(test_duration - required_duration), milliseconds(10));
}

BOOST_AUTO_TEST_CASE(timer_fast_forward)
{
    using namespace maidsafe;
    using clock = std::chrono::steady_clock;

    asio::io_service ios;

    crux::detail::timer timer(ios);

    auto start_time = clock::now();
    auto end_time   = start_time;

    auto timer_handler = [&]() {
        end_time = clock::now();
        timer.stop();
    };

    timer.set_period(milliseconds(100));
    timer.set_handler(timer_handler);
    timer.fast_forward();

    ios.run();

    auto test_duration     = end_time - start_time;

    BOOST_REQUIRE_LE(test_duration, milliseconds(10));
}

BOOST_AUTO_TEST_CASE(timer_start_and_fast_forward)
{
    using namespace maidsafe;
    using clock = std::chrono::steady_clock;

    asio::io_service ios;

    crux::detail::timer timer(ios);

    auto start_time = clock::now();
    auto end_time   = start_time;

    auto timer_handler = [&]() {
        end_time = clock::now();
        timer.stop();
    };

    timer.set_period(milliseconds(100));
    timer.set_handler(timer_handler);
    timer.start();
    timer.fast_forward();

    ios.run();

    auto test_duration     = end_time - start_time;

    BOOST_REQUIRE_LE(test_duration, milliseconds(10));
}

BOOST_AUTO_TEST_CASE(timer_start_then_fast_forward)
{
    using namespace maidsafe;
    using clock = std::chrono::steady_clock;

    asio::io_service ios;

    crux::detail::timer timer(ios);

    auto start_time = clock::now();
    auto end_time   = start_time;

    bool was_called       = false;
    bool was_called_twice = false;

    auto timer_handler = [&]() {
        if (!was_called) {
            was_called = true;
            timer.fast_forward();
        }
        else {
            was_called_twice = true;
            end_time = clock::now();
            timer.stop();
        }
    };

    timer.set_period(milliseconds(100));
    timer.set_handler(timer_handler);
    timer.start();

    ios.run();

    auto test_duration     = end_time - start_time;

    BOOST_REQUIRE(was_called_twice);
    BOOST_REQUIRE_LE(abs(test_duration - milliseconds(100)), milliseconds(10));
}
