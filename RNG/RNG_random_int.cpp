#include "rng.hpp"
#include "rng_internal.hpp"
#include <limits>
#include <random>

int ft_random_int(void)
{
    ft_init_random_engine();
    std::uniform_int_distribution<int> distribution(0, std::numeric_limits<int>::max());
    int random_value;
    int lock_error;
    int unlock_error;

    lock_error = g_random_engine_mutex.lock();
    if (lock_error != 0)
        return (0);
    random_value = distribution(g_random_engine);
    unlock_error = g_random_engine_mutex.unlock();
    if (unlock_error != 0)
        return (0);
    return (random_value);
}
