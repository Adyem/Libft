#include "rng.hpp"
#include "rng_internal.hpp"

int ft_random_int(void)
{
    ft_init_srand();
    int result = 0;
    while (result == 0)
        result = rand();
    return (result);
}
