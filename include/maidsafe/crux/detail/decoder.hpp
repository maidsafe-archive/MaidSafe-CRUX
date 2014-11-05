#ifndef MAIDSAFE_CRUX_DETAIL_DECODER_HPP
#define MAIDSAFE_CRUX_DETAIL_DECODER_HPP

#include <cstdint>

namespace maidsafe
{
namespace crux
{
namespace detail
{

class decoder
{
public:
    using const_iterator = const char *;

    decoder(const_iterator begin, const_iterator end);

    std::int32_t get_int32() const;

private:
    struct
    {
        const_iterator begin;
        const_iterator end;
    } current;
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

inline decoder::decoder(const_iterator begin,
                        const_iterator end)
{
    current.begin = begin;
    current.end = end;
}

} // namespace detail
} // namespace crux
} // namespace maidsafe

#endif // MAIDSAFE_CRUX_DETAIL_DECODER_HPP
