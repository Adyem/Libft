#include "rng.hpp"

int64_t rng_seed_value(const char *seed_string)
{
    uint64_t random_value;
    uint64_t fallback_value;

    if (seed_string != ft_nullptr)
        return (ft_hash_string31(seed_string));
    if (rng_secure_uint64(&random_value) == FT_ERR_SUCCESS)
        return (static_cast<int64_t>(random_value));
    fallback_value = static_cast<uint64_t>(ft_random_seed(ft_nullptr));
    fallback_value <<= 32;
    fallback_value |= static_cast<uint64_t>(ft_random_seed(ft_nullptr));
    return (static_cast<int64_t>(fallback_value));
}
