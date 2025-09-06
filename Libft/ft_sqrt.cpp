#include "libft.hpp"

double ft_sqrt(double number)
{
    double guess;
    double next_guess;

    if (number < 0)
        return (-1.0);
    if (ft_fabs(number) < 1e-12)
        return (0.0);
    guess = number;
    while (1)
    {
        next_guess = 0.5 * (guess + number / guess);
        if (ft_fabs(next_guess - guess) < 0.000001)
            break;
        guess = next_guess;
    }
    return (next_guess);
}
