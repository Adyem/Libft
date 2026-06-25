#include "math.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "math_interval.hpp"

double math_indeterminate(void)
{
    double zero;

    zero = 0.0;
    return (zero / zero);
}
