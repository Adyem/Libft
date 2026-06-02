#include "math.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "math_interval.hpp"

double math_negative_infinity(void)
{
    double negative_one;
    double zero;

    negative_one = -1.0;
    zero = 0.0;
    return (negative_one / zero);
}
