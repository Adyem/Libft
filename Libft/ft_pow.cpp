#include "libft.hpp"

double ft_pow(double base, int exponent)
{
    double result = 1.0;
    int    exp = exponent;

    if (exp < 0)
    {
        base = 1.0 / base;
        exp = -exp;
    }
    while (exp > 0)
    {
        if (exp % 2 == 1)
            result *= base;
        base *= base;
        exp /= 2;
    }
    return (result);
}
