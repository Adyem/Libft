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

unsigned long ft_strtoul(const char *input_string, char **end_pointer, int numeric_base)
{
    const char *current_character = input_string;
    int sign_value = 1;
    unsigned long accumulated_value = 0;
    int digit_value;
    bool overflow_detected = false;
    bool digit_processed = false;
    unsigned long base_value;
    unsigned long limit_value;
    int error_code = FT_ERR_SUCCESSS;

    if (current_character == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        if (end_pointer != ft_nullptr)
            *end_pointer = ft_nullptr;
        return (0UL);
    }
    if (numeric_base != 0 && (numeric_base < 2 || numeric_base > 36))
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        if (end_pointer != ft_nullptr)
            *end_pointer = const_cast<char *>(input_string);
        return (0UL);
    }
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
    limit_value = FT_ULONG_MAX;
    while ((digit_value = ft_digit_value(*current_character)) >= 0
            && digit_value < numeric_base)
    {
        unsigned long digit_as_unsigned;
        unsigned long limit_division;
        unsigned long limit_remainder;

        digit_as_unsigned = static_cast<unsigned long>(digit_value);
        digit_processed = true;
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
    if (!digit_processed)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        if (end_pointer != ft_nullptr)
            *end_pointer = const_cast<char *>(input_string);
        return (0UL);
    }
    if (end_pointer)
        *end_pointer = const_cast<char *>(current_character);
    if (overflow_detected)
        error_code = FT_ERR_OUT_OF_RANGE;
    if (sign_value < 0)
    {
        if (overflow_detected)
        {
            ft_global_error_stack_push(error_code);
            return (limit_value);
        }
        unsigned long negated_value = 0UL - accumulated_value;
        ft_global_error_stack_push(error_code);
        return (negated_value);
    }
    ft_global_error_stack_push(error_code);
    return (accumulated_value);
}
