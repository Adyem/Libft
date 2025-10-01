#include "rng.hpp"
#include "rng_internal.hpp"
#include <limits>
#include <random>
#include "../Errno/errno.hpp"
#include "../PThread/unique_lock.hpp"

int ft_random_int(void)
{
    ft_init_srand();
    std::uniform_int_distribution<int> distribution(0, std::numeric_limits<int>::max());
    int random_value;

    ft_unique_lock<pt_mutex> guard(g_random_engine_mutex);
    if (guard.get_error() != ER_SUCCESS)
        return (0);
    random_value = distribution(g_random_engine);
    ft_errno = ER_SUCCESS;
    return (random_value);
}
