#include "math.hpp"
#include "../Errno/errno.hpp"

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
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (math_nan());
    }
    ft_errno = FT_ER_SUCCESSS;
    return (sin_value / cos_value);
}
