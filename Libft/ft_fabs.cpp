#include "libft.hpp"

double ft_fabs(double number)
{
    if (number < 0.0)
        return (-number);
    return (number);
}
