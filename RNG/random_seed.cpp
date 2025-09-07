#include "RNG.hpp"
#include <cstdint>
#include <random>

int ft_random_seed(const char *seed_string)
{
    if (seed_string != ft_nullptr)
    {
        uint32_t hash = 2166136261u;
        while (*seed_string)
        {
            hash ^= static_cast<unsigned char>(*seed_string++);
            hash *= 16777619u;
        }
        return (static_cast<int>(hash));
    }
    std::random_device random_device;
    return (static_cast<int>(random_device()));
}
