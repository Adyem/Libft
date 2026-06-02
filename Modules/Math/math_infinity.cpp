#include "math.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "math_interval.hpp"

double math_infinity(void)
{
    double zero;
    double one;

    zero = 0.0;
    one = 1.0;
    return (one / zero);
}
