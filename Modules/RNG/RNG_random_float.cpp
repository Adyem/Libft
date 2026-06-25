#include "rng.hpp"
#include "rng_internal.hpp"
#include "../Errno/errno.hpp"
#include <random>
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Template/pair.hpp"

float ft_random_float(void)
{
    ft_init_random_engine();
    std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
    float random_value;
    int32_t lock_error;
    int32_t unlock_error;

    lock_error = rng_lock_random_engine_mutex();
    if (lock_error != FT_ERR_SUCCESS)
        return (0.0f);
    random_value = distribution(g_random_engine);
    unlock_error = rng_unlock_random_engine_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
        return (0.0f);
    return (random_value);
}
