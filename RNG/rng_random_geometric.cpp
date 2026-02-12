#include <limits>
#include "rng.hpp"
#include "rng_internal.hpp"
#include "../Errno/errno.hpp"

int ft_random_geometric(double success_probability)
{
    int trial_count;
    double random_value;

    ft_init_random_engine();
    if (success_probability <= 0.0)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (0);
    }
    if (success_probability > 1.0)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (0);
    }
    const double probability_epsilon = std::numeric_limits<double>::epsilon();
    if ((1.0 - success_probability) <= probability_epsilon)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESS);
        return (1);
    }
    trial_count = 1;
    while (1)
    {
        random_value = static_cast<double>(ft_random_float());
        int error_code = ft_global_error_stack_drop_last_error();
        if (error_code != FT_ERR_SUCCESS)
        {
            ft_global_error_stack_push(error_code);
            return (0);
        }
        if (random_value < success_probability)
        {
            ft_global_error_stack_push(FT_ERR_SUCCESS);
            return (trial_count);
        }
        trial_count = trial_count + 1;
    }
    ft_global_error_stack_push(FT_ERR_INTERNAL);
    return (0);
}
