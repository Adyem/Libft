#include "rng.hpp"
#include "rng_internal.hpp"
#include <random>

float ft_random_gamma(float shape, float scale)
{
    float sample_value;
    std::gamma_distribution<float> distribution;
    int lock_error;
    int unlock_error;

    ft_init_random_engine();
    if (shape <= 0.0f || scale <= 0.0f)
        return (0.0f);
    distribution = std::gamma_distribution<float>(shape, scale);
    lock_error = g_random_engine_mutex.lock();
    if (lock_error != 0)
        return (0.0f);
    sample_value = distribution(g_random_engine);
    unlock_error = g_random_engine_mutex.unlock();
    if (unlock_error != 0)
        return (0.0f);
    return (sample_value);
}
