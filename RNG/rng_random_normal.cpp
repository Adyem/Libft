#include "rng.hpp"
#include "rng_internal.hpp"
#include "../Math/math.hpp"

float ft_random_normal(void)
{
    float uniform_one;
    float uniform_two;
    const float pi_value = 3.14159265358979323846f;
    float radius;
    float angle;
    float result;
    double log_value;
    double sqrt_value;
    double cosine_value;

    ft_init_random_engine();
    uniform_one = ft_random_float();
    if (uniform_one < 0.0000000001f)
        uniform_one = 0.0000000001f;
    uniform_two = ft_random_float();

    log_value = math_log(uniform_one);
    sqrt_value = math_sqrt(-2.0 * log_value);
    radius = static_cast<float>(sqrt_value);
    angle = 2.0f * pi_value * uniform_two;
    cosine_value = math_cos(angle);
    result = radius * static_cast<float>(cosine_value);
    return (result);
}
