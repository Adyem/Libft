#include "math.hpp"
#include "../Errno/errno.hpp"

double math_fabs(double number)
{
    ft_errno = FT_ER_SUCCESSS;
    if (math_signbit(number))
        return (-number);
    return (number);
}
