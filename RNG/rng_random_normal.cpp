#include "rng.hpp"
#include "rng_internal.hpp"
#include <cmath>

float ft_random_normal(void)
{
    ft_init_srand();
    float uniform_one = ft_random_float();
    float uniform_two = ft_random_float();
    const float pi_value = 3.14159265358979323846f;
    float radius = std::sqrt(-2.0f * std::log(uniform_one));
    float angle = 2.0f * pi_value * uniform_two;
    float result = radius * std::cos(angle);
    return (result);
}
