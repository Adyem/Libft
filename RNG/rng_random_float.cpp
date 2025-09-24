#include "rng.hpp"
#include "rng_internal.hpp"

float ft_random_float(void)
{
    ft_init_srand();
    float random_value;

    random_value = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    return (random_value);
}
