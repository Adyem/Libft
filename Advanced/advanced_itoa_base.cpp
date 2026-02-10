#include <cstddef>
#include "advanced.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"

static int calculate_length(int number, int base)
{
    int length = 0;
    unsigned int value = (number < 0) ? -static_cast<unsigned int>(number) : static_cast<unsigned int>(number);
    if (value == 0)
        return (1);
    while (value != 0)
    {
        value /= base;
        length += 1;
    }
    return (length);
}

char *adv_itoa_base(int number, int base)
{
    if (base < 2 || base > 16)
        return (ft_nullptr);
    const char digits[] = "0123456789ABCDEF";
    int is_negative = (number < 0 && base == 10) ? 1 : 0;
    unsigned int value = (number < 0) ? -static_cast<unsigned int>(number) : static_cast<unsigned int>(number);
    int length = calculate_length(number, base);
    char *result = static_cast<char *>(cma_malloc(static_cast<ft_size_t>(length + is_negative + 1)));
    if (result == ft_nullptr)
        return (ft_nullptr);
    result[length + is_negative] = '\0';
    while (length > 0)
    {
        result[length + is_negative - 1] = digits[value % base];
        value /= base;
        length -= 1;
    }
    if (is_negative)
        result[0] = '-';
    return (result);
}
