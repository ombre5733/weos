#include "testutils.hpp"

std::uint32_t random()
{
    // Produce a pseudo-random number in the range [1, 2147483646].
    static std::uint32_t x = 1;
    x = (static_cast<std::uint64_t>(x) * 16807UL) % 2147483647UL;

    return x - 1;
}
