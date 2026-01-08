#include "math.hpp"
#include "../Errno/errno.hpp"

double math_fabs(double number)
{
    if (math_signbit(number))
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (-number);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (number);
}
