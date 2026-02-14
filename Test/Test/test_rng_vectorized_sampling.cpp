#include "../test_internal.hpp"
#include "../../RNG/rng.hpp"
#include "../../RNG/rng_internal.hpp"
#include "../../System_utils/test_runner.hpp"
#include <limits>

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_rng_vectorized_int_matches_scalar,
        "ft_random_int_vector matches scalar draws for identical seeds")
{
    int vector_values[8];
    int scalar_values[8];
    size_t value_index;

    ft_seed_random_engine(123456789u);
    if (ft_random_int_vector(0, std::numeric_limits<int>::max(), vector_values, 8) != 0)
        return (0);
    ft_seed_random_engine(123456789u);
    value_index = 0;
    while (value_index < 8)
    {
        scalar_values[value_index] = ft_random_int();
        value_index += 1;
    }
    value_index = 0;
    while (value_index < 8)
    {
        if (vector_values[value_index] != scalar_values[value_index])
            return (0);
        value_index += 1;
    }
    return (1);
}

FT_TEST(test_rng_vectorized_float_matches_scalar,
        "ft_random_float_vector matches scalar draws within tolerance")
{
    float vector_values[8];
    float scalar_values[8];
    size_t value_index;
    float difference;

    ft_seed_random_engine(362436069u);
    if (ft_random_float_vector(vector_values, 8) != 0)
        return (0);
    ft_seed_random_engine(362436069u);
    value_index = 0;
    while (value_index < 8)
    {
        scalar_values[value_index] = ft_random_float();
        value_index += 1;
    }
    value_index = 0;
    while (value_index < 8)
    {
        difference = vector_values[value_index] - scalar_values[value_index];
        if (difference < 0.0f)
            difference = -difference;
        if (difference > 0.000001f)
            return (0);
        value_index += 1;
    }
    return (1);
}
