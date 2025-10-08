#include <limits>
#include "rng.hpp"
#include "rng_internal.hpp"
#include "../Errno/errno.hpp"

int ft_random_geometric(double success_probability)
{
    int trial_count;
    double random_value;

    ft_init_srand();
    if (success_probability <= 0.0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (0);
    }
    if (success_probability > 1.0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (0);
    }
    const double probability_epsilon = std::numeric_limits<double>::epsilon();
    if ((1.0 - success_probability) <= probability_epsilon)
    {
        ft_errno = ER_SUCCESS;
        return (1);
    }
    trial_count = 1;
    while (1)
    {
        random_value = static_cast<double>(ft_random_float());
        if (ft_errno != ER_SUCCESS)
            return (0);
        if (random_value < success_probability)
        {
            ft_errno = ER_SUCCESS;
            return (trial_count);
        }
        trial_count = trial_count + 1;
    }
    return (0);
}
