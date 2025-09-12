#include "rng.hpp"
#include "rng_internal.hpp"

int ft_random_geometric(double success_probability)
{
    int trial_count;
    double random_value;

    ft_init_srand();
    if (success_probability <= 0.0)
        return (0);
    if (success_probability >= 1.0)
    {
        if (success_probability > 1.0)
            return (0);
        return (1);
    }
    trial_count = 1;
    while (1)
    {
        random_value = static_cast<double>(ft_random_float());
        if (random_value < success_probability)
            return (trial_count);
        trial_count = trial_count + 1;
    }
    return (0);
}
