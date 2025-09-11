#include "math.hpp"

double ft_tan(double value)
{
    double sin_value;
    double cos_value;

    sin_value = ft_sin(value);
    cos_value = math_cos(value);
    return (sin_value / cos_value);
}
