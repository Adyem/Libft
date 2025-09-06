#include "libft.hpp"

typedef union u_double_bits
{
    double              d;
    unsigned long long  u;
}   t_double_bits;

int ft_signbit(double number)
{
    t_double_bits bits;

    bits.d = number;
    if ((bits.u >> 63) != 0)
        return (1);
    return (0);
}
