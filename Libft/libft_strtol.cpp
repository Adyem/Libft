#include "libft.hpp"
#include "limits.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

static int ft_digit_value(char character)
{
    if (character >= '0' && character <= '9')
        return (character - '0');
    if (character >= 'a' && character <= 'z')
        return (character - 'a' + 10);
    if (character >= 'A' && character <= 'Z')
        return (character - 'A' + 10);
    return (-1);
}

long ft_strtol(const char *input_string, char **end_pointer, int numeric_base)
{
    const char *current_character = input_string;
    long sign_value = 1;
    unsigned long accumulated_value = 0;
    int digit_value;
    bool overflow_detected = false;
    unsigned long positive_limit;
    unsigned long negative_limit;
    unsigned long base_value;

    if (current_character == ft_nullptr)
    {
        ft_errno = FT_EINVAL;
        if (end_pointer != ft_nullptr)
            *end_pointer = ft_nullptr;
        return (0L);
    }
    ft_errno = ER_SUCCESS;
    while (*current_character == ' ' || (*current_character >= '\t'
                && *current_character <= '\r'))
        ++current_character;
    if (*current_character == '+' || *current_character == '-')
    {
        if (*current_character == '-')
            sign_value = -1;
        ++current_character;
    }
    if (numeric_base == 0)
    {
        if (*current_character == '0')
        {
            if (current_character[1] == 'x' || current_character[1] == 'X')
            {
                numeric_base = 16;
                current_character += 2;
            }
            else
                numeric_base = 8;
        }
        else
            numeric_base = 10;
    }
    else if (numeric_base == 16 && current_character[0] == '0'
             && (current_character[1] == 'x' || current_character[1] == 'X'))
        current_character += 2;
    base_value = static_cast<unsigned long>(numeric_base);
    positive_limit = static_cast<unsigned long>(FT_LONG_MAX);
    negative_limit = positive_limit + 1UL;
    while ((digit_value = ft_digit_value(*current_character)) >= 0
            && digit_value < numeric_base)
    {
        unsigned long limit_value;
        unsigned long limit_division;
        unsigned long limit_remainder;
        unsigned long digit_as_unsigned;

        digit_as_unsigned = static_cast<unsigned long>(digit_value);
        limit_value = positive_limit;
        if (sign_value < 0)
            limit_value = negative_limit;
        if (base_value == 0)
            break;
        limit_division = limit_value / base_value;
        limit_remainder = limit_value % base_value;
        if (accumulated_value > limit_division
                || (accumulated_value == limit_division
                    && digit_as_unsigned > limit_remainder))
        {
            overflow_detected = true;
            accumulated_value = limit_value;
            break;
        }
        accumulated_value = accumulated_value * base_value
                          + digit_as_unsigned;
        ++current_character;
    }
    if (end_pointer)
        *end_pointer = const_cast<char *>(current_character);
    if (overflow_detected)
        ft_errno = FT_ERANGE;
    if (sign_value < 0)
    {
        if (accumulated_value > positive_limit)
            return (FT_LONG_MIN);
        long result = static_cast<long>(accumulated_value);
        return (-result);
    }
    long result = static_cast<long>(accumulated_value);
    return (result);
}
