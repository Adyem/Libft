#include "math.hpp"

double ft_fabs(double number)
{
    if (ft_signbit(number))
        return (-number);
    return (number);
}
