#include "rng.hpp"
#include "rng_internal.hpp"
#include <random>

float ft_random_float(void)
{
    ft_init_random_engine();
    std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
    float random_value;
    int lock_error;
    int unlock_error;

    lock_error = rng_lock_random_engine_mutex();
    if (lock_error != 0)
        return (0.0f);
    random_value = distribution(g_random_engine);
    unlock_error = rng_unlock_random_engine_mutex();
    if (unlock_error != 0)
        return (0.0f);
    return (random_value);
}
