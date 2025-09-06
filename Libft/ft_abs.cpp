#include "libft.hpp"

int ft_abs(int number)
{
    if (number < 0)
        return (-number);
    return (number);
}

long ft_abs(long number)
{
    if (number < 0)
        return (-number);
    return (number);
}

long long ft_abs(long long number)
{
    if (number < 0)
        return (-number);
    return (number);
}
