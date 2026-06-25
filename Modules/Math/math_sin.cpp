#include "math.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "math_interval.hpp"

double ft_sin(double value)
{
    double angle;
    double term;
    double result;
    int32_t    iteration;
    double sign;

    angle = value;
    while (angle > 3.14159265358979323846)
        angle -= 6.28318530717958647692;
    while (angle < -3.14159265358979323846)
        angle += 6.28318530717958647692;
    term = angle;
    result = angle;
    iteration = 1;
    sign = -1.0;
    while (math_fabs(term) > 0.0000000001)
    {
        term = term * angle * angle / ((2.0 * iteration) * (2.0 * iteration + 1.0));
        result = result + sign * term;
        sign = -sign;
        iteration = iteration + 1;
    }
    return (result);
}
