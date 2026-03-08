#include "math.hpp"

int32_t math_clamp(int32_t value, int32_t minimum, int32_t maximum)
{
    if (value < minimum)
        return (minimum);
    if (value > maximum)
        return (maximum);
    return (value);
}

double math_clamp(double value, double minimum, double maximum)
{
    if (value < minimum)
        return (minimum);
    if (value > maximum)
        return (maximum);
    return (value);
}
