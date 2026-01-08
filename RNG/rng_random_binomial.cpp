#include <limits>

#include "rng.hpp"
#include "rng_internal.hpp"
#include "../Errno/errno.hpp"

int ft_random_binomial(int trial_count, double success_probability)
{
    int trial_index;
    int success_count;
    double random_value;

    ft_init_random_engine();
    if (trial_count < 0)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (0);
    }
    if (success_probability < 0.0)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (0);
    }
    if (trial_count == 0)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (0);
    }
    if (success_probability > 1.0)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (0);
    }
    const double probability_epsilon = std::numeric_limits<double>::epsilon();
    if (success_probability <= probability_epsilon)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (0);
    }
    if ((1.0 - success_probability) <= probability_epsilon)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (trial_count);
    }
    trial_index = 0;
    success_count = 0;
    while (trial_index < trial_count)
    {
        random_value = static_cast<double>(ft_random_float());
        int error_code = ft_global_error_stack_pop_newest();
        if (error_code != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(error_code);
            return (0);
        }
        if (random_value < success_probability)
            success_count = success_count + 1;
        trial_index = trial_index + 1;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (success_count);
}
