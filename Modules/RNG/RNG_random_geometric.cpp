#include <limits>
#include "rng.hpp"
#include "rng_internal.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Template/pair.hpp"

int32_t ft_random_geometric(double success_probability)
{
    int32_t trial_count;
    int32_t zero_value;
    double random_value;

    zero_value = FT_ERR_SUCCESS;
    ft_init_random_engine();
    if (success_probability <= 0.0)
        return (FT_ERR_INVALID_ARGUMENT);
    if (success_probability > 1.0)
        return (FT_ERR_INVALID_ARGUMENT);
    const double PROBABILITY_EPSILON = std::numeric_limits<double>::epsilon();
    if ((1.0 - success_probability) <= PROBABILITY_EPSILON)
        return (1);
    trial_count = 1;
    while (1)
    {
        random_value = static_cast<double>(ft_random_float());
        if (random_value < success_probability)
            return (trial_count);
        trial_count = trial_count + 1;
    }
    return (zero_value);
}
