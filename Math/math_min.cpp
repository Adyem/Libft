#include "math.hpp"
#include "../Errno/errno.hpp"

int math_min(int first_number, int second_number)
{
    if (first_number < second_number)
        return (first_number);
    return (second_number);
}

long math_min(long first_number, long second_number)
{
    if (first_number < second_number)
        return (first_number);
    return (second_number);
}

long long math_min(long long first_number, long long second_number)
{
    if (first_number < second_number)
        return (first_number);
    return (second_number);
}

double math_min(double first_number, double second_number)
{
    if (first_number < second_number)
        return (first_number);
    return (second_number);
}
