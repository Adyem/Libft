#include "roll.hpp"
#include "math_internal.hpp"

int32_t    math_roll_check_number_next(char *string, int32_t index)
{
    if (!(string[index + 1] == '+' || string[index + 1] == '-' ||
            (string[index + 1] >= '0' && string[index + 1] <= '9')))
        return (1);
    if (string[index + 1] == '+' || string[index + 1] == '-')
        if (!(string[index + 2] >= '0' && string[index + 2] <= '9'))
            return (1);
    return (0);
}

int32_t    math_roll_check_number_previous(char *string, int32_t index)
{
    if (index == 1)
        if (!(string[index - 1] >= '0' && string[index - 1] <= '9'))
            return (1);
    return (0);
}

int32_t    math_roll_check_character(char character)
{
    if (character == 0)
        return (0);
    if (character == '-' || character == '+' || character == '/' || character == '*')
        return (0);
    return (1);
}
