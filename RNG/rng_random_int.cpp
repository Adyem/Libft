#include "rng.hpp"
#include "rng_internal.hpp"
#include <limits>
#include <mutex>
#include <random>

int ft_random_int(void)
{
    ft_init_srand();
    std::uniform_int_distribution<int> distribution(0, std::numeric_limits<int>::max());
    int random_value;

    {
        std::lock_guard<std::mutex> guard(g_random_engine_mutex);
        random_value = distribution(g_random_engine);
    }
    return (random_value);
}
