#include "rng.hpp"
#include "rng_internal.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/unique_lock.hpp"
#include <random>

float ft_random_gamma(float shape, float scale)
{
    float sample_value;
    std::gamma_distribution<float> distribution;

    ft_init_random_engine();
    if (shape <= 0.0f || scale <= 0.0f)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (0.0f);
    }
    distribution = std::gamma_distribution<float>(shape, scale);
    {
        ft_unique_lock<pt_mutex> guard(g_random_engine_mutex);

        if (guard.get_error() != ER_SUCCESS)
            return (0.0f);
        sample_value = distribution(g_random_engine);
    }
    ft_errno = ER_SUCCESS;
    return (sample_value);
}
