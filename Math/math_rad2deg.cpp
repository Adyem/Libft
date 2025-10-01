#include "math.hpp"
#include "../Errno/errno.hpp"

double math_rad2deg(double radians)
{
    const double pi_value = 3.14159265358979323846;

    ft_errno = ER_SUCCESS;
    return (radians * 180.0 / pi_value);
}
