#include "math.hpp"
#include "../Errno/errno.hpp"

double math_exp(double value)
{
    double current_term;
    double result;
    int    iteration;

    ft_errno = FT_ERR_SUCCESSS;
    current_term = 1.0;
    result = 1.0;
    iteration = 1;
    while (math_fabs(current_term) > 0.0000000001)
    {
        current_term *= value / iteration;
        result += current_term;
        ++iteration;
    }
    return (result);
}
