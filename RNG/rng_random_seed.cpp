#include "rng.hpp"
#include <cstdint>
#include <random>
#include "../Errno/errno.hpp"

uint32_t ft_random_seed(const char *seed_string)
{
    if (seed_string != ft_nullptr)
    {
        uint32_t hash = 2166136261u;
        while (*seed_string)
        {
            hash ^= static_cast<unsigned char>(*seed_string++);
            hash *= 16777619u;
        }
        ft_errno = ER_SUCCESS;
        return (hash);
    }
    std::random_device random_device;
    std::uniform_int_distribution<uint32_t> distribution;
    uint32_t random_value;

    random_value = distribution(random_device);
    ft_errno = ER_SUCCESS;
    return (random_value);
}
