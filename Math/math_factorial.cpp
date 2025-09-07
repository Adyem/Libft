#include "math.hpp"

int math_factorial(int number)
{
    if (number < 0)
        return (0);
    if (number == 0 || number == 1)
        return (1);
    return (number * math_factorial(number - 1));
}

long math_factorial(long number)
{
    if (number < 0)
        return (0);
    if (number == 0 || number == 1)
        return (1);
    return (number * math_factorial(number - 1));
}

long long math_factorial(long long number)
{
    if (number < 0)
        return (0);
    if (number == 0 || number == 1)
        return (1);
    return (number * math_factorial(number - 1));
}

