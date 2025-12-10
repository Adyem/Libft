#include "math.hpp"
#include "../Errno/errno.hpp"

double math_cos(double value)
{
    double angle;
    double term;
    double result;
    int    iteration;
    double sign;

    angle = value;
    while (angle > 3.14159265358979323846)
        angle -= 6.28318530717958647692;
    while (angle < -3.14159265358979323846)
        angle += 6.28318530717958647692;
    term = 1.0;
    result = 1.0;
    iteration = 1;
    sign = -1.0;
    while (math_fabs(term) > 0.0000000001)
    {
        term = term * angle * angle / ((2.0 * iteration - 1.0) * (2.0 * iteration));
        result = result + sign * term;
        sign = -sign;
        iteration = iteration + 1;
    }
    ft_errno = FT_ER_SUCCESSS;
    return (result);
}
