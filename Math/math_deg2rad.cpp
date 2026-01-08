#include "math.hpp"
#include "../Errno/errno.hpp"

double math_deg2rad(double degrees)
{
    const double pi_value = 3.14159265358979323846;

    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (degrees * pi_value / 180.0);
}
