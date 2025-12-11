#include "math.hpp"

double math_nan(void)
{
    ft_errno = FT_ERR_SUCCESSS;
    double zero;

    zero = 0.0;
    return (zero / zero);
}
