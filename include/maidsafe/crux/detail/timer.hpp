///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2014 MaidSafe.net Limited
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
///////////////////////////////////////////////////////////////////////////////

#ifndef MAIDSAFE_CRUX_DETAIL_PERIODIC_TIMER_HPP
#define MAIDSAFE_CRUX_DETAIL_PERIODIC_TIMER_HPP

#include <boost/asio/steady_timer.hpp>

namespace maidsafe
{
namespace crux
{
namespace detail
{

class timer {
public:
    using handler_type  = std::function<void()>;
    using timer_type    = boost::asio::steady_timer;
    using duration_type = timer_type::duration;

public:
    timer(boost::asio::io_service&);

    template<class HandlerType>
    timer(boost::asio::io_service&, HandlerType&&);

    void set_period(const duration_type&);

    template<typename HandlerType> void set_handler(HandlerType&& handler);

    // The next time the handler is called will be
    // period_duration from now.
    void start();

    // Handler will no longer be executed (unless start or
    // fast_forward is called again).
    void stop();

    // The next handler invocation will happen as soon as possible
    // (but never inside this fast_forward function).
    void fast_forward();

    ~timer();

private:
    void do_handle_tick();
    void do_start();

private:
    enum state_type {
        stopped,
        running,
        executing,
        canceling_to_stop,
        canceling_to_start,
        canceling_to_ff
    };

    state_type    state;
    duration_type period_duration;
    timer_type    asio_timer;
    handler_type  handler;

    std::shared_ptr<bool> was_destroyed;
};

} // namespace detail
} // namespace crux
} // namespace maidsafe

namespace maidsafe
{
namespace crux
{
namespace detail
{

inline
timer::timer(boost::asio::io_service& ios)
    : state(stopped)
    , asio_timer(ios)
    , was_destroyed(std::make_shared<bool>(false))
{}

template<class HandlerType>
timer::timer( boost::asio::io_service& ios
                              , HandlerType&& handler)
    : state(stopped)
    , asio_timer(ios)
    , handler(std::forward<HandlerType>(handler))
    , was_destroyed(std::make_shared<bool>(false))
{}

inline
timer::~timer() {
    stop();
    *was_destroyed = true;
}

inline
void timer::set_period(const duration_type& duration) {
    period_duration = duration;
}

template<typename HandlerType>
void timer::set_handler(HandlerType&& handler) {
    this->handler = std::forward<HandlerType>(handler);
}

inline void timer::start() {
    switch (state) {
        case stopped: do_start();
                      break;
        case running: stop();
                      start();
                      break;
        case executing: do_start();
                        break;
        case canceling_to_stop: state = canceling_to_start;
                                break;
        case canceling_to_start: break;
        case canceling_to_ff: state = canceling_to_start;
                              break;
    }
}

inline void timer::stop() {
    switch (state) {
        case stopped: break;
        case running: state = canceling_to_stop;
                      asio_timer.cancel();
                      break;
        case executing: state = stopped;
                        break;
        case canceling_to_stop: break;
        case canceling_to_start: state = canceling_to_stop;
                                 break;
        case canceling_to_ff: state = canceling_to_stop;
                              break;
    }
}

inline void timer::fast_forward() {
    start();
    stop();
    state = canceling_to_ff;
}

inline
void timer::do_start() {
    state = running;
    asio_timer.expires_from_now(period_duration);

    auto was_destroyed_copy = was_destroyed;

    asio_timer.async_wait(
            [=](const boost::system::error_code&) {
              if (*was_destroyed_copy) return;
              do_handle_tick();
            });
}

inline void timer::do_handle_tick() {
    switch (state) {
        case stopped: return;
        case running: break;
        case executing: assert(0); break;
        case canceling_to_stop: state = stopped;
                                return;
        case canceling_to_start: do_start();
                                 return;
        case canceling_to_ff: break;
    }

    state = executing;

    if (handler) {
        // The handler execution may destroy this timer
        // object or it may set the handler to a new value.
        auto was_destroyed_copy = was_destroyed;

        auto local_handler = std::move(handler);

        local_handler();

        // Has the handler destroyed this object?
        if (*was_destroyed_copy) return;

        if (!handler) {
            handler = std::move(local_handler);
        }
    }

    if (state == executing) {
        state = stopped;
    }
}

} // namespace detail
} // namespace crux
} // namespace maidsafe

#endif // ifndef MAIDSAFE_CRUX_DETAIL_PERIODIC_TIMER_HPP

