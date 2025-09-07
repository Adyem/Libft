#include "math.hpp"

int math_lcm(int first_number, int second_number)
{
    int greatest_common_divisor;

    greatest_common_divisor = math_gcd(first_number, second_number);
    if (greatest_common_divisor == 0)
        return (0);
    return (math_abs(first_number / greatest_common_divisor * second_number));
}

long math_lcm(long first_number, long second_number)
{
    long greatest_common_divisor;

    greatest_common_divisor = math_gcd(first_number, second_number);
    if (greatest_common_divisor == 0)
        return (0);
    return (math_abs(first_number / greatest_common_divisor * second_number));
}

long long math_lcm(long long first_number, long long second_number)
{
    long long greatest_common_divisor;

    greatest_common_divisor = math_gcd(first_number, second_number);
    if (greatest_common_divisor == 0)
        return (0);
    return (math_abs(first_number / greatest_common_divisor * second_number));
}

