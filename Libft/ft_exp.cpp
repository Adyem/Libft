#include "libft.hpp"

double ft_exp(double x)
{
    double term;
    double sum;
    int    n;

    term = 1.0;
    sum = 1.0;
    n = 1;
    while (ft_fabs(term) > 0.0000000001)
    {
        term *= x / n;
        sum += term;
        ++n;
    }
    return (sum);
}
