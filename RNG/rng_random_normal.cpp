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

    ft_init_srand();
    uniform_one = ft_random_float();
    if (uniform_one < 0.0000000001f)
        uniform_one = 0.0000000001f;
    uniform_two = ft_random_float();
    radius = static_cast<float>(math_sqrt(-2.0 * math_log(uniform_one)));
    angle = 2.0f * pi_value * uniform_two;
    result = radius * static_cast<float>(math_cos(angle));
    return (result);
}
