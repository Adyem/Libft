#include "math.hpp"

double math_fabs(double number)
{
    if (math_signbit(number))
        return (-number);
    return (number);
}
