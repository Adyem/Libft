#include "rng.hpp"
#include "rng_internal.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/unique_lock.hpp"
#include <random>

float ft_random_beta(float alpha, float beta)
{
    std::gamma_distribution<float> alpha_distribution;
    std::gamma_distribution<float> beta_distribution;
    float alpha_sample;
    float beta_sample;
    float sum;
    float result;

    ft_init_random_engine();
    if (alpha <= 0.0f || beta <= 0.0f)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (0.0f);
    }
    alpha_distribution = std::gamma_distribution<float>(alpha, 1.0f);
    beta_distribution = std::gamma_distribution<float>(beta, 1.0f);
    {
        ft_unique_lock<pt_mutex> guard(g_random_engine_mutex);

        if (guard.get_error() != ER_SUCCESS)
            return (0.0f);
        alpha_sample = alpha_distribution(g_random_engine);
        beta_sample = beta_distribution(g_random_engine);
    }
    sum = alpha_sample + beta_sample;
    if (sum <= 0.0f)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return (0.0f);
    }
    result = alpha_sample / sum;
    if (result < 0.0f)
        result = 0.0f;
    if (result > 1.0f)
        result = 1.0f;
    ft_errno = ER_SUCCESS;
    return (result);
}
