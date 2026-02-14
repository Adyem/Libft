#ifndef RNG_TEST_UTILS_HPP
#define RNG_TEST_UTILS_HPP

#include "../../Basic/basic.hpp"
#include "../../RNG/rng.hpp"
#include "../../RNG/rng_internal.hpp"
#include "../../System_utils/system_utils.hpp"

static uint32_t    ft_rng_test_seed_value(uint32_t default_seed, const char *context)
{
    char        *environment_value;
    uint32_t    override_seed;

    environment_value = ft_getenv("FT_RNG_TEST_SEED");
    if (environment_value == ft_nullptr || *environment_value == '\0')
        return (default_seed);
    override_seed = ft_random_seed(environment_value);
    if (context != ft_nullptr && *context != '\0')
    {
        uint32_t    context_hash;

        context_hash = ft_random_seed(context);
        override_seed = override_seed ^ context_hash;
    }
    override_seed = override_seed ^ default_seed;
    return (override_seed);
}

static void    ft_rng_test_seed_engine(uint32_t default_seed, const char *context)
{
    uint32_t    seed_value;

    seed_value = ft_rng_test_seed_value(default_seed, context);
    ft_seed_random_engine(seed_value);
    return ;
}

[[maybe_unused]]
static void    ft_rng_test_seed_engine(uint32_t default_seed)
{
    ft_rng_test_seed_engine(default_seed, ft_nullptr);
    return ;
}

#endif
