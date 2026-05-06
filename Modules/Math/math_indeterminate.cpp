#include "math.hpp"

double math_indeterminate(void)
{
    double zero;

    zero = 0.0;
    return (zero / zero);
}
