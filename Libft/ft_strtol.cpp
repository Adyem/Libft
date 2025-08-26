#include "libft.hpp"

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
    while ((digit_value = ft_digit_value(*current_character)) >= 0
			&& digit_value < numeric_base)
    {
        accumulated_value = accumulated_value * static_cast<unsigned long>(numeric_base)
                          + static_cast<unsigned long>(digit_value);
        ++current_character;
    }
    if (end_pointer)
        *end_pointer = const_cast<char *>(current_character);
    return ((sign_value < 0)
        ? -static_cast<long>(accumulated_value)
        :  static_cast<long>(accumulated_value));
}
