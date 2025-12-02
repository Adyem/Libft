#include "math.hpp"

typedef union u_double_bits
{
    double              double_value;
    unsigned long long  unsigned_value;
}   t_double_bits;

int math_signbit(double number)
{
    t_double_bits bit_structure;

    if (math_isnan(number))
        return (0);
    bit_structure.double_value = number;
    if ((bit_structure.unsigned_value >> 63) != 0)
        return (1);
    return (0);
}
