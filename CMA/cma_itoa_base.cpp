#include "CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"

static int    calculate_length(int number, int base)
{
    int length = 0;
    unsigned int absolute_value;
    if (number < 0)
        absolute_value = static_cast<unsigned int>(-number);
    else
        absolute_value = static_cast<unsigned int>(number);
    if (absolute_value == 0)
        return (1);
    while (absolute_value)
    {
        absolute_value /= base;
        length++;
    }
    return (length);
}

char    *cma_itoa_base(int number, int base)
{
    const char digits[] = "0123456789ABCDEF";
    int is_negative = 0;
    int length;
    char *result_string;
    unsigned int absolute_value;

    if (base < 2 || base > 16)
    {
        ft_errno = FT_EINVAL;
        return (ft_nullptr);
    }
    if (number < 0 && base == 10)
        is_negative = 1;
    if (number < 0)
        absolute_value = static_cast<unsigned int>(-number);
    else
        absolute_value = static_cast<unsigned int>(number);
    length = calculate_length(number, base);
    result_string = static_cast<char*>(cma_malloc(length + is_negative + 1));
    if (!result_string)
        return (ft_nullptr);
    result_string[length + is_negative] = '\0';
    while (length > 0)
    {
        result_string[length + is_negative - 1] = digits[absolute_value % base];
        absolute_value /= base;
        length--;
    }
    if (is_negative)
        result_string[0] = '-';
    return (result_string);
}
