#include "rng.hpp"
#include "rng_internal.hpp"
#include <cmath>

float ft_random_exponential(float lambda_value)
{
    ft_init_srand();
    if (lambda_value <= 0.0f)
        return (0.0f);
    float uniform_value = ft_random_float();
    float result = -std::log(uniform_value) / lambda_value;
    return (result);
}
