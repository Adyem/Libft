#include "rng.hpp"
#include "rng_internal.hpp"
#include <limits>
#include <random>
#include "../Errno/errno.hpp"
#include "../PThread/unique_lock.hpp"

int ft_random_int(void)
{
    ft_init_random_engine();
    std::uniform_int_distribution<int> distribution(0, std::numeric_limits<int>::max());
    int random_value;

    ft_unique_lock<pt_mutex> guard(g_random_engine_mutex);
    int error_code = ft_global_error_stack_pop_newest();
    if (error_code != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(error_code);
        return (0);
    }
    random_value = distribution(g_random_engine);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (random_value);
}
