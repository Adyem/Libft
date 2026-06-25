#include "math.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "math_interval.hpp"

double math_deg2rad(double degrees)
{
    const double pi_value = 3.14159265358979323846;

    return (degrees * pi_value / 180.0);
}
