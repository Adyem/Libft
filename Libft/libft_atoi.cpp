#include "libft.hpp"
#include "limits.hpp"

int    ft_atoi(const char *string)
{
    int    index = 0;
    int    sign = 1;
    unsigned long long result = 0;
    const unsigned long long positive_limit = static_cast<unsigned long long>(FT_INT_MAX);
    const unsigned long long negative_limit = static_cast<unsigned long long>(FT_INT_MAX) + 1ULL;

    while (string[index] == ' ' || ((string[index] >= '\t')
                && (string[index] <= '\r')))
        index++;
    if (string[index] == '+' || string[index] == '-')
    {
        if (string[index] == '-')
            sign = -1;
        index++;
    }
    while (string[index] && ((string[index] >= '0')
                && (string[index] <= '9')))
    {
        int    digit = string[index] - '0';

        if (sign == 1)
        {
            if (result > positive_limit / 10)
                return (FT_INT_MAX);
            if (result == positive_limit / 10
                && static_cast<unsigned long long>(digit) > positive_limit % 10)
                return (FT_INT_MAX);
        }
        else
        {
            if (result > negative_limit / 10)
                return (FT_INT_MIN);
            if (result == negative_limit / 10
                && static_cast<unsigned long long>(digit) > negative_limit % 10)
                return (FT_INT_MIN);
        }
        result = result * 10 + static_cast<unsigned long long>(digit);
        index++;
    }
    if (sign == -1)
    {
        long long signed_result = -static_cast<long long>(result);

        return (static_cast<int>(signed_result));
    }
    return (static_cast<int>(result));
}
