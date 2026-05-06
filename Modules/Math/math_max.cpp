#include "math.hpp"
#include "../Errno/errno.hpp"

int32_t math_max(int32_t first_number, int32_t second_number)
{
    if (first_number > second_number)
        return (first_number);
    return (second_number);
}

int64_t math_max(int64_t first_number, int64_t second_number)
{
    if (first_number > second_number)
        return (first_number);
    return (second_number);
}

double math_max(double first_number, double second_number)
{
    if (first_number > second_number)
        return (first_number);
    return (second_number);
}
