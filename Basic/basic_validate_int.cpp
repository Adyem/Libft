//#include "limits.hpp" // FT_INT32_* defined there
#include "basic.hpp"

int32_t ft_validate_int(const char *input)
{
    if (!input)
        return (FT_FAILURE);

    const int32_t maximum_value = FT_INT32_MAX;
    const int32_t minimum_value = FT_INT32_MIN;
    int32_t index = 0;
    int32_t sign = 1;
    if (input[index] == '+' || input[index] == '-')
    {
        if (input[index] == '-')
            sign = -1;
        ++index;
    }
    if (input[index] == '\0')
        return (FT_FAILURE);

    int64_t accumulated = 0;
    while (input[index] != '\0')
    {
        char character = input[index];
        if (character < '0' || character > '9')
            return (FT_FAILURE);
        int32_t digit = character - '0';
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
