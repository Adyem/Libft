#include "rng.hpp"
#include "rng_internal.hpp"
#include <limits>
#include <random>

int32_t ft_random_int(void)
{
    std::random_device random_device;
    int32_t fallback_value;

    fallback_value = static_cast<int32_t>(random_device()
            & static_cast<uint32_t>(std::numeric_limits<int32_t>::max()));
    ft_init_random_engine();
    std::uniform_int_distribution<int32_t> distribution(0, std::numeric_limits<int32_t>::max());
    int32_t random_value;
    int32_t lock_error;
    int32_t unlock_error;

    lock_error = rng_lock_random_engine_mutex();
    if (lock_error != FT_ERR_SUCCESS)
        return (fallback_value);
    random_value = distribution(g_random_engine);
    unlock_error = rng_unlock_random_engine_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
        return (fallback_value);
    return (random_value);
}
