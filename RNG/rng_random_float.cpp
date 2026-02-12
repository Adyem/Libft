#include "rng.hpp"
#include "rng_internal.hpp"
#include <random>
#include "../Errno/errno.hpp"
#include "../PThread/unique_lock.hpp"

float ft_random_float(void)
{
    ft_init_random_engine();
    std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
    float random_value;

    ft_unique_lock<pt_mutex> guard(g_random_engine_mutex);
    int error_code = ft_global_error_stack_drop_last_error();
    if (error_code != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(error_code);
        return (0.0f);
    }
    random_value = distribution(g_random_engine);
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (random_value);
}
