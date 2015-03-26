///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2014 MaidSafe.net Limited
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
///////////////////////////////////////////////////////////////////////////////

#ifndef MAIDSAFE_CRUX_DETAIL_SERVICE_HPP
#define MAIDSAFE_CRUX_DETAIL_SERVICE_HPP

#include <maidsafe/crux/detail/config.hpp>

#include <memory>
#include <map>
#include <random>
#include <mutex>
#include <boost/asio/io_service.hpp>
#include <maidsafe/crux/endpoint.hpp>

namespace maidsafe
{
namespace crux
{
namespace detail
{
class multiplexer;

class MAIDSAFE_CRUX_DECL service : public boost::asio::io_service::service
{
    using endpoint_type = crux::endpoint;
    using multiplexer_map = std::map< endpoint_type, std::weak_ptr<detail::multiplexer> >;

public:
    static boost::asio::io_service::id id;

    explicit service(boost::asio::io_service& io);

    // Get or create the multiplexer that owns a local endpoint
    std::shared_ptr<detail::multiplexer> add(endpoint_type local_endpoint);
    void remove(const endpoint_type& local_endpoint);

    std::uint32_t random();

    // Required by boost::asio::basic_io_object
    struct implementation_type {};
    void construct(implementation_type& /*impl*/) {}
    void destroy(implementation_type&) {}
    // Required for move construction
    void move_construct(implementation_type&, implementation_type&) {}
    void move_assign(implementation_type&, service&, implementation_type&) {}

private:
    virtual void shutdown_service() {}

private:
    std::mutex      mutex;
    multiplexer_map multiplexers;

    std::mt19937 generator;
    std::uniform_int_distribution<std::uint32_t> distribution;
};

} // namespace detail
} // namespace crux
} // namespace maidsafe

#include <limits>
#include <maidsafe/crux/detail/multiplexer.hpp>

namespace maidsafe
{
namespace crux
{
namespace detail
{

inline service::service(boost::asio::io_service& io)
    : boost::asio::io_service::service(io),
      distribution(std::numeric_limits<std::uint32_t>::min(),
                   std::numeric_limits<std::uint32_t>::max())
{
    std::random_device device;
    generator.seed(device());
}

inline std::shared_ptr<detail::multiplexer> service::add(endpoint_type local_endpoint)
{
    using next_layer_type = detail::multiplexer::next_layer_type;

    std::lock_guard<std::mutex> guard(mutex);

    std::shared_ptr<detail::multiplexer> result;
    auto where = multiplexers.lower_bound(local_endpoint);
    if ((where == multiplexers.end()) || (multiplexers.key_comp()(local_endpoint, where->first)))
    {
        // Multiplexer for local endpoint does not exists

        next_layer_type socket(get_io_service(), local_endpoint);

        // local_endpoint changes if ephemeral port is used.
        local_endpoint = socket.local_endpoint();

        result = detail::multiplexer::create(std::move(socket));

        where = multiplexers.insert(where,
                                    multiplexer_map::value_type
                                        ( local_endpoint
                                        , result));
    }
    else
    {
        result = where->second.lock();
        if (!result)
        {
            // This can happen if an acceptor has failed
            // Reassign if empty
            next_layer_type socket(get_io_service(), local_endpoint);

            assert(local_endpoint == socket.local_endpoint());

            result = detail::multiplexer::create(std::move(socket));

            where->second = result;
        }
    }
    return result;
}

inline void service::remove(const endpoint_type& local_endpoint)
{
    std::lock_guard<std::mutex> guard(mutex);

    auto where = multiplexers.find(local_endpoint);
    if (where != multiplexers.end())
    {
        // Only remove if multiplexer is unused
        if (!where->second.lock())
        {
            multiplexers.erase(where);
        }
    }
}

inline std::uint32_t service::random()
{
    return distribution(generator);
}

} // namespace detail
} // namespace crux
} // namespace maidsafe

#endif // MAIDSAFE_CRUX_DETAIL_SERVICE_HPP
