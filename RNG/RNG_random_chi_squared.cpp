#include "rng.hpp"
#include "rng_internal.hpp"
#include <random>

float ft_random_chi_squared(float degrees_of_freedom)
{
    float sample_value;
    std::gamma_distribution<float> distribution;
    int lock_error;
    int unlock_error;

    ft_init_random_engine();
    if (degrees_of_freedom <= 0.0f)
        return (0.0f);
    distribution = std::gamma_distribution<float>(degrees_of_freedom * 0.5f, 2.0f);
    lock_error = rng_lock_random_engine_mutex();
    if (lock_error != 0)
        return (0.0f);
    sample_value = distribution(g_random_engine);
    unlock_error = rng_unlock_random_engine_mutex();
    if (unlock_error != 0)
        return (0.0f);
    if (sample_value < 0.0f)
        sample_value = 0.0f;
    return (sample_value);
}
