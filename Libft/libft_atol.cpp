#include "libft.hpp"
#include "limits.hpp"
#include "../CPP_class/class_nullptr.hpp"

long ft_atol(const char *string)
{
    long index = 0;
    long sign = 1;
    long result = 0;
    const long positive_limit_divider = FT_LONG_MAX / 10;
    const long positive_limit_remainder = FT_LONG_MAX % 10;
    const long negative_limit_divider = FT_LONG_MIN / 10;
    const long negative_limit_remainder = -(FT_LONG_MIN % 10);

    if (string == ft_nullptr)
        return (0);
    while (string[index] == ' ' || (string[index] >= '\t' && string[index] <= '\r'))
        index++;
    if (string[index] == '+' || string[index] == '-')
    {
        if (string[index] == '-')
            sign = -1;
        index++;
    }
    while (string[index] >= '0' && string[index] <= '9')
    {
        int digit = string[index] - '0';

        if (sign == 1)
        {
            if (result > positive_limit_divider)
                return (FT_LONG_MAX);
            if (result == positive_limit_divider && digit > positive_limit_remainder)
                return (FT_LONG_MAX);
            result = result * 10 + digit;
        }
        else
        {
            if (result < negative_limit_divider)
                return (FT_LONG_MIN);
            if (result == negative_limit_divider && digit > negative_limit_remainder)
                return (FT_LONG_MIN);
            result = result * 10 - digit;
        }
        index++;
    }
    return (result);
}
