#include "math.hpp"
#include "../Errno/errno.hpp"

double math_sqrt(double number)
{
    double guess;
    double next_guess;
    double difference;
    int    iteration_count;
    int    max_iterations;

    if (math_isnan(number))
    {
        ft_errno = FT_EINVAL;
        return (math_nan());
    }
    if (number < 0)
    {
        ft_errno = FT_EINVAL;
        return (-1.0);
    }
    if (math_fabs(number) < 1e-12)
    {
        ft_errno = ER_SUCCESS;
        return (0.0);
    }
    ft_errno = ER_SUCCESS;
    guess = number;
    iteration_count = 0;
    max_iterations = 1000;
    while (iteration_count < max_iterations)
    {
        next_guess = 0.5 * (guess + number / guess);
        if (math_isnan(next_guess))
        {
            ft_errno = FT_ERANGE;
            return (math_nan());
        }
        difference = math_fabs(next_guess - guess);
        if (difference < 0.000001)
            return (next_guess);
        guess = next_guess;
        iteration_count += 1;
    }
    ft_errno = FT_ERANGE;
    return (math_nan());
}
