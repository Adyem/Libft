#include "rng.hpp"
#include "rng_internal.hpp"

int ft_random_binomial(int trial_count, double success_probability)
{
    int trial_index;
    int success_count;
    double random_value;

    ft_init_srand();
    if (trial_count <= 0)
        return (0);
    if (success_probability <= 0.0)
        return (0);
    if (success_probability >= 1.0)
    {
        if (success_probability > 1.0)
            return (0);
        return (trial_count);
    }
    trial_index = 0;
    success_count = 0;
    while (trial_index < trial_count)
    {
        random_value = static_cast<double>(ft_random_float());
        if (random_value < success_probability)
            success_count = success_count + 1;
        trial_index = trial_index + 1;
    }
    return (success_count);
}
