#include "limits.hpp"

int ft_validate_int(const char *input)
{
    long number;
    int index;
    int sign;
    long signed_number;

    number = 0;
    index = 0;
    sign = 1;
    if (input[index] == '+' || input[index] == '-')
        index++;
    if (!input[index])
        return (1);
    if (input[0] == '-')
        sign = -1;
    while (input[index])
    {
        if (input[index] >= '0' && input[index] <= '9')
        {
            number = (number * 10) + input[index] - '0';
            signed_number = sign * number;
            if (signed_number < FT_INT_MIN || signed_number > FT_INT_MAX)
                return (2);
            index++;
        }
        else
            return (3);
    }
    return (0);
}
