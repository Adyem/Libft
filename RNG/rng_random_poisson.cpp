#include "rng.hpp"
#include "rng_internal.hpp"
#include "../Errno/errno.hpp"
#include "../Math/math.hpp"

int ft_random_poisson(double lambda_value)
{
    double limit_value;
    double product_value;
    int count_value;
    double random_value;

    ft_init_srand();
    if (lambda_value <= 0.0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (0);
    }
    limit_value = math_exp(-lambda_value);
    if (ft_errno != ER_SUCCESS)
        return (0);
    product_value = 1.0;
    count_value = 0;
    while (product_value > limit_value)
    {
        random_value = static_cast<double>(ft_random_float());
        if (ft_errno != ER_SUCCESS)
            return (0);
        product_value = product_value * random_value;
        count_value = count_value + 1;
    }
    ft_errno = ER_SUCCESS;
    return (count_value - 1);
}
