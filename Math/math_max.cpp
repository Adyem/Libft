#include "math.hpp"
#include "../Errno/errno.hpp"

int math_max(int first_number, int second_number)
{
    ft_errno = FT_ERR_SUCCESSS;
    if (first_number > second_number)
        return (first_number);
    return (second_number);
}

long math_max(long first_number, long second_number)
{
    ft_errno = FT_ERR_SUCCESSS;
    if (first_number > second_number)
        return (first_number);
    return (second_number);
}

long long math_max(long long first_number, long long second_number)
{
    ft_errno = FT_ERR_SUCCESSS;
    if (first_number > second_number)
        return (first_number);
    return (second_number);
}

double math_max(double first_number, double second_number)
{
    ft_errno = FT_ERR_SUCCESSS;
    if (first_number > second_number)
        return (first_number);
    return (second_number);
}

