#include "libft.hpp"

static double ft_abs_double(double number)
{
    if (number < 0)
        return (-number);
    return (number);
}

double ft_sqrt(double number)
{
    double guess;
    double next_guess;

    if (number < 0)
        return (-1.0);
    if (number == 0)
        return (0.0);
    guess = number;
    while (1)
    {
        next_guess = 0.5 * (guess + number / guess);
        if (ft_abs_double(next_guess - guess) < 0.000001)
            break;
        guess = next_guess;
    }
    return (next_guess);
}
