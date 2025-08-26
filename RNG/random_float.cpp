#include "RNG.hpp"
#include "RNG_internal.hpp"

float ft_random_float(void)
{
    ft_init_srand();
    float result = 0.0f;
    while (result <= 1e-12f)
        result = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    return (result);
}
