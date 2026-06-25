#include "rng.hpp"
#include "rng_internal.hpp"
#include "../Errno/errno.hpp"
#include "../Math/math.hpp"
#include "../Basic/limits.hpp"
#include "../Math/math_interval.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Template/pair.hpp"

int32_t ft_random_poisson(double lambda_value)
{
    double limit_value;
    double product_value;
    int32_t count_value;
    double random_value;

    ft_init_random_engine();
    if (lambda_value <= 0.0)
        return (FT_ERR_INVALID_ARGUMENT);
    limit_value = math_exp(-lambda_value);
    product_value = 1.0;
    count_value = 0;
    while (product_value > limit_value)
    {
        random_value = static_cast<double>(ft_random_float());
        product_value = product_value * random_value;
        count_value = count_value + 1;
    }
    return (count_value - 1);
}
