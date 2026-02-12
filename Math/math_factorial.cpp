#include "math.hpp"
#include "../Errno/errno.hpp"
#include <climits>

int math_factorial(int number)
{
    int result;
    int current_number;

    if (number < 0)
    {
        return (0);
    }
    result = 1;
    current_number = 2;
    while (current_number <= number)
    {
        if (result > INT_MAX / current_number)
        {
            return (0);
        }
        result *= current_number;
        current_number += 1;
    }
    return (result);
}

long math_factorial(long number)
{
    long result;
    long current_number;

    if (number < 0)
    {
        return (0);
    }
    result = 1;
    current_number = 2;
    while (current_number <= number)
    {
        if (result > LONG_MAX / current_number)
        {
            return (0);
        }
        result *= current_number;
        current_number += 1;
    }
    return (result);
}

long long math_factorial(long long number)
{
    long long result;
    long long current_number;

    if (number < 0)
    {
        return (0);
    }
    result = 1;
    current_number = 2;
    while (current_number <= number)
    {
        if (result > LLONG_MAX / current_number)
        {
            return (0);
        }
        result *= current_number;
        current_number += 1;
    }
    return (result);
}
