#include "rng.hpp"
#include "rng_internal.hpp"
#include "../Math/math.hpp"

float ft_random_exponential(float lambda_value)
{
    float uniform_value;
    float result;

    ft_init_srand();
    if (lambda_value <= 0.0f)
        return (0.0f);
    uniform_value = ft_random_float();
    if (uniform_value < 0.0000000001f)
        uniform_value = 0.0000000001f;
    result = static_cast<float>(-math_log(uniform_value)) / lambda_value;
    return (result);
}
