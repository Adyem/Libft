#include "RNG.hpp"
#include <cstdint>
#include <random>

int ft_random_seed(const char *seed_str)
{
    if (seed_str != ft_nullptr)
    {
        uint32_t hash = 2166136261u;
        while (*seed_str)
        {
            hash ^= static_cast<unsigned char>(*seed_str++);
            hash *= 16777619u;
        }
        return static_cast<int>(hash);
    }
    std::random_device rd;
    return static_cast<int>(rd());
}
