#include "rng.hpp"
#include "rng_internal.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/unique_lock.hpp"
#include <random>

float ft_random_chi_squared(float degrees_of_freedom)
{
    float sample_value;
    std::gamma_distribution<float> distribution;

    ft_init_random_engine();
    if (degrees_of_freedom <= 0.0f)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (0.0f);
    }
    distribution = std::gamma_distribution<float>(degrees_of_freedom * 0.5f, 2.0f);
    {
        ft_unique_lock<pt_mutex> guard(g_random_engine_mutex);
        int error_code = ft_global_error_stack_drop_last_error();

        if (error_code != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(error_code);
            return (0.0f);
        }
        sample_value = distribution(g_random_engine);
    }
    if (sample_value < 0.0f)
        sample_value = 0.0f;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (sample_value);
}
