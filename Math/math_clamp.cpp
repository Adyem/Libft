#include "math.hpp"

int math_clamp(int value, int minimum, int maximum)
{
    if (value < minimum)
        return (minimum);
    if (value > maximum)
        return (maximum);
    return (value);
}
