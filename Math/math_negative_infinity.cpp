#include "math.hpp"

double math_negative_infinity(void)
{
    double negative_one;
    double zero;

    negative_one = -1.0;
    zero = 0.0;
    return (negative_one / zero);
}
