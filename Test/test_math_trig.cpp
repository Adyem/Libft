#include "../Math/math.hpp"

int test_ft_sin_zero(void)
{
    double result;

    result = ft_sin(0.0);
    if (math_fabs(result) < 0.000001)
        return (1);
    return (0);
}

int test_ft_sin_ninety(void)
{
    double result;

    result = ft_sin(math_deg2rad(90.0));
    if (math_fabs(result - 1.0) < 0.000001)
        return (1);
    return (0);
}

int test_ft_tan_zero(void)
{
    double result;

    result = ft_tan(0.0);
    if (math_fabs(result) < 0.000001)
        return (1);
    return (0);
}

int test_ft_tan_forty_five(void)
{
    double result;

    result = ft_tan(math_deg2rad(45.0));
    if (math_fabs(result - 1.0) < 0.000001)
        return (1);
    return (0);
}
