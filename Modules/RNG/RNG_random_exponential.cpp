#include "rng.hpp"
#include "rng_internal.hpp"
#include "../Math/math.hpp"

float ft_random_exponential(float lambda_value)
{
    float uniform_value;
    float result;

    ft_init_random_engine();
    if (lambda_value <= 0.0f)
        return (0.0f);
    uniform_value = ft_random_float();
    if (uniform_value < 0.0000000001f)
        uniform_value = 0.0000000001f;
    double log_value;

    log_value = math_log(uniform_value);
    result = static_cast<float>(-log_value) / lambda_value;
    return (result);
}
