#include "rng.hpp"
#include "rng_internal.hpp"

int ft_random_int(void)
{
    ft_init_srand();
    int random_value;

    random_value = rand();
    return (random_value);
}
