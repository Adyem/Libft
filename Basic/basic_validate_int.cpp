//#include "limits.hpp" // FT_INT_* defined there
#include "basic.hpp"

int ft_validate_int(const char *input)
{
    if (!input)
        return (FT_FAILURE);

    const long maximum_value = static_cast<long>(FT_INT_MAX);
    const long minimum_value = static_cast<long>(FT_INT_MIN);
    int index = 0;
    int sign = 1;
    if (input[index] == '+' || input[index] == '-')
    {
        if (input[index] == '-')
            sign = -1;
        ++index;
    }
    if (input[index] == '\0')
        return (FT_FAILURE);

    long accumulated = 0;
    while (input[index] != '\0')
    {
        char character = input[index];
        if (character < '0' || character > '9')
            return (FT_FAILURE);
        int digit = character - '0';
        if (sign == 1)
        {
            if (accumulated > (maximum_value - digit) / 10)
                return (FT_FAILURE);
            accumulated = accumulated * 10 + digit;
        }
        else
        {
            if (accumulated < (minimum_value + digit) / 10)
                return (FT_FAILURE);
            accumulated = accumulated * 10 - digit;
        }
        ++index;
    }
    return (FT_SUCCESS);
}
