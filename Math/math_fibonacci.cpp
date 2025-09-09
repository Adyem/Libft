#include "math.hpp"

int math_fibonacci(int number)
{
    if (number <= 0)
        return (0);
    if (number == 1)
        return (1);
    return (math_fibonacci(number - 1) + math_fibonacci(number - 2));
}

long math_fibonacci(long number)
{
    if (number <= 0)
        return (0);
    if (number == 1)
        return (1);
    return (math_fibonacci(number - 1) + math_fibonacci(number - 2));
}

long long math_fibonacci(long long number)
{
    if (number <= 0)
        return (0);
    if (number == 1)
        return (1);
    return (math_fibonacci(number - 1) + math_fibonacci(number - 2));
}
