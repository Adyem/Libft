#include "math.hpp"

double math_nan(void)
{
    double zero;

    zero = 0.0;
    return (zero / zero);
}
