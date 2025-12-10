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
    if (guard.get_error() != FT_ER_SUCCESSS)
        return (0.0f);
    random_value = distribution(g_random_engine);
    ft_errno = FT_ER_SUCCESSS;
    return (random_value);
}
