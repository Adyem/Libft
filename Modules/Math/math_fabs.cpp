#include "math.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "math_interval.hpp"

double math_fabs(double number)
{
    if (math_signbit(number))
    {
        return (-number);
    }
    return (number);
}
