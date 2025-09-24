#include "rng.hpp"
#include "rng_internal.hpp"
#include <mutex>
#include <random>

float ft_random_float(void)
{
    ft_init_srand();
    std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
    float random_value;

    {
        std::lock_guard<std::mutex> guard(g_random_engine_mutex);
        random_value = distribution(g_random_engine);
    }
    return (random_value);
}
