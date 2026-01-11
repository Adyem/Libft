#include "CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

static int    itoa_length(int number)
{
    int        length;

    length = 0;
    if (number == 0)
        return (1);
    while (number)
    {
        number /= 10;
        length++;
    }
    return (length);
}

static char *fill_digits(char *characters, unsigned int number, int start_index)
{
    while (start_index >= 0)
    {
        characters[start_index] = static_cast<char>((number % 10) + '0');
        number /= 10;
        start_index--;
    }
    return (characters);
}

static char    *convert_int(int number, int is_negative, int *error_code)
{
    int                length;
    char            *result;
    unsigned int    absolute_value;

    length = itoa_length(number);
    result = static_cast<char *>(cma_malloc(length + 1 + is_negative));
    *error_code = ft_global_error_stack_pop_newest();
    if (!result)
    {
        return (ft_nullptr);
    }
    if (number < 0)
        absolute_value = -number;
    else
        absolute_value = number;
    result[length + is_negative] = '\0';
    if (is_negative == 0)
        result = fill_digits(result, absolute_value, length - 1);
    else
        result = fill_digits(result, absolute_value, length);
    if (is_negative == 1)
        result[0] = '-';
    return (result);
}

char    *cma_itoa(int number)
{
    int    is_negative;
    char    *result;
    int     error_code;

    if (number > 0)
        is_negative = 0;
    else if (number == -2147483648)
    {
        result = cma_strdup("-2147483648");
        error_code = ft_global_error_stack_pop_newest();
        if (!result)
        {
            ft_global_error_stack_push(error_code);
            return (ft_nullptr);
        }
        error_code = FT_ERR_SUCCESSS;
        ft_global_error_stack_push(error_code);
        return (result);
    }
    else if (number == 0)
    {
        result = cma_strdup("0");
        error_code = ft_global_error_stack_pop_newest();
        if (!result)
        {
            ft_global_error_stack_push(error_code);
            return (ft_nullptr);
        }
        error_code = FT_ERR_SUCCESSS;
        ft_global_error_stack_push(error_code);
        return (result);
    }
    else
        is_negative = 1;
    result = convert_int(number, is_negative, &error_code);
    if (!result)
    {
        ft_global_error_stack_push(error_code);
        return (ft_nullptr);
    }
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (result);
}
