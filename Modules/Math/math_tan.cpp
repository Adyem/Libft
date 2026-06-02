#include "math.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "math_interval.hpp"

double ft_tan(double value)
{
    double sin_value;
    double cos_value;
    double epsilon;

    epsilon = 0.0000000001;
    sin_value = ft_sin(value);
    cos_value = math_cos(value);
    if (math_fabs(cos_value) < epsilon)
    {
        return (math_nan());
    }
    return (sin_value / cos_value);
}
