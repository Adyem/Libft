#include "math.hpp"
#include "../Errno/errno.hpp"

static int math_gcd_normalized_int(int first_number, int second_number)
{
    int remainder_value;

    remainder_value = 0;
    while (second_number != 0)
    {
        remainder_value = first_number % second_number;
        first_number = second_number;
        second_number = remainder_value;
    }
    return (first_number);
}

static long math_gcd_normalized_long(long first_number, long second_number)
{
    long remainder_value;

    remainder_value = 0;
    while (second_number != 0)
    {
        remainder_value = first_number % second_number;
        first_number = second_number;
        second_number = remainder_value;
    }
    return (first_number);
}

static long long math_gcd_normalized_long_long(long long first_number, long long second_number)
{
    long long remainder_value;

    remainder_value = 0;
    while (second_number != 0)
    {
        remainder_value = first_number % second_number;
        first_number = second_number;
        second_number = remainder_value;
    }
    return (first_number);
}

int math_gcd(int first_number, int second_number)
{
    int result_value;

    first_number = math_abs(first_number);
    second_number = math_abs(second_number);
    result_value = math_gcd_normalized_int(first_number, second_number);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result_value);
}

long math_gcd(long first_number, long second_number)
{
    long result_value;

    first_number = math_abs(first_number);
    second_number = math_abs(second_number);
    result_value = math_gcd_normalized_long(first_number, second_number);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result_value);
}

long long math_gcd(long long first_number, long long second_number)
{
    long long result_value;

    first_number = math_abs(first_number);
    second_number = math_abs(second_number);
    result_value = math_gcd_normalized_long_long(first_number, second_number);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result_value);
}
