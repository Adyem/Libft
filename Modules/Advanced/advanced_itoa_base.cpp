#include <cstddef>
#include "advanced.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"

static int32_t calculate_length(int32_t number, int32_t base)
{
    int32_t length = 0;
    uint32_t value = 0;
    if (number < 0)
        value = -static_cast<uint32_t>(number);
    else
        value = static_cast<uint32_t>(number);
    if (value == 0)
        return (1);
    while (value != 0)
    {
        value /= base;
        length += 1;
    }
    return (length);
}

char *adv_itoa_base(int32_t number, int32_t base)
{
    if (base < 2 || base > 16)
        return (ft_nullptr);
    const char digits[] = "0123456789ABCDEF";
    int32_t is_negative = 0;
    if (number < 0 && base == 10)
        is_negative = 1;
    uint32_t value = 0;
    if (number < 0)
        value = -static_cast<uint32_t>(number);
    else
        value = static_cast<uint32_t>(number);
    int32_t length = calculate_length(number, base);
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
