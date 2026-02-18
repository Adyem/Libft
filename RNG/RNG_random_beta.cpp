#include "rng.hpp"
#include "rng_internal.hpp"
#include <random>

float ft_random_beta(float alpha, float beta)
{
    std::gamma_distribution<float> alpha_distribution;
    std::gamma_distribution<float> beta_distribution;
    float alpha_sample;
    float beta_sample;
    float sum;
    float result;
    int lock_error;
    int unlock_error;

    ft_init_random_engine();
    if (alpha <= 0.0f || beta <= 0.0f)
        return (0.0f);
    alpha_distribution = std::gamma_distribution<float>(alpha, 1.0f);
    beta_distribution = std::gamma_distribution<float>(beta, 1.0f);
    lock_error = rng_lock_random_engine_mutex();
    if (lock_error != 0)
        return (0.0f);
    alpha_sample = alpha_distribution(g_random_engine);
    beta_sample = beta_distribution(g_random_engine);
    unlock_error = rng_unlock_random_engine_mutex();
    if (unlock_error != 0)
        return (0.0f);
    sum = alpha_sample + beta_sample;
    if (sum <= 0.0f)
        return (0.0f);
    result = alpha_sample / sum;
    if (result < 0.0f)
        result = 0.0f;
    if (result > 1.0f)
        result = 1.0f;
    return (result);
}
