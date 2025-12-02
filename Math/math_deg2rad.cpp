#include "math.hpp"
#include "../Errno/errno.hpp"

double math_deg2rad(double degrees)
{
    const double pi_value = 3.14159265358979323846;

    ft_errno = ER_SUCCESS;
    return (degrees * pi_value / 180.0);
}
