#include "math.hpp"
#include "../Errno/errno.hpp"

int math_gcd(int first_number, int second_number)
{
    first_number = math_abs(first_number);
    second_number = math_abs(second_number);
    if (second_number == 0)
    {
        ft_errno = ER_SUCCESS;
        return (first_number);
    }
    return (math_gcd(second_number, first_number % second_number));
}

long math_gcd(long first_number, long second_number)
{
    first_number = math_abs(first_number);
    second_number = math_abs(second_number);
    if (second_number == 0)
    {
        ft_errno = ER_SUCCESS;
        return (first_number);
    }
    return (math_gcd(second_number, first_number % second_number));
}

long long math_gcd(long long first_number, long long second_number)
{
    first_number = math_abs(first_number);
    second_number = math_abs(second_number);
    if (second_number == 0)
    {
        ft_errno = ER_SUCCESS;
        return (first_number);
    }
    return (math_gcd(second_number, first_number % second_number));
}

