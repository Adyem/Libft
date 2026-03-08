#include "math.hpp"
#include "../Errno/errno.hpp"
#include <climits>

int32_t math_factorial(int32_t number)
{
    int32_t result;
    int32_t current_number;

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

int64_t math_factorial(int64_t number)
{
    int64_t result;
    int64_t current_number;

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
