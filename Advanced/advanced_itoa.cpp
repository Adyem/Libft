#include "advanced.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"

static int32_t itoa_length(int32_t number)
{
    int32_t length = 0;
    if (number == 0)
        return (1);
    while (number != 0)
    {
        number /= 10;
        length += 1;
    }
    return (length);
}

static char *fill_digits(char *characters, uint32_t value, int32_t index)
{
    while (index >= 0)
    {
        characters[index] = static_cast<char>(value % 10 + '0');
        value /= 10;
        index -= 1;
    }
    return (characters);
}

static char *convert_int(int32_t number, int32_t is_negative)
{
    int32_t length = itoa_length(number);
    char *result = static_cast<char *>(cma_malloc(static_cast<ft_size_t>(length + 1 + is_negative)));
    if (result == ft_nullptr)
        return (ft_nullptr);
    uint32_t absolute_value = 0;
    if (number < 0)
        absolute_value = -static_cast<uint32_t>(number);
    else
        absolute_value = static_cast<uint32_t>(number);
    result[length + is_negative] = '\0';
    if (is_negative == 0)
        fill_digits(result, absolute_value, length - 1);
    else
    {
        fill_digits(result, absolute_value, length);
        result[0] = '-';
    }
    return (result);
}

char *adv_itoa(int32_t number)
{
    if (number == 0)
    {
        char *result = static_cast<char *>(cma_malloc(2));
        if (result == ft_nullptr)
            return (ft_nullptr);
        result[0] = '0';
        result[1] = '\0';
        return (result);
    }
    int32_t is_negative = 0;
    if (number < 0)
        is_negative = 1;
    return (convert_int(number, is_negative));
}
