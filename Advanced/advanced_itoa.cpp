#include "advanced.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"

static int itoa_length(int number)
{
    int length = 0;
    if (number == 0)
        return (1);
    while (number != 0)
    {
        number /= 10;
        length += 1;
    }
    return (length);
}

static char *fill_digits(char *characters, unsigned int value, int index)
{
    while (index >= 0)
    {
        characters[index] = static_cast<char>(value % 10 + '0');
        value /= 10;
        index -= 1;
    }
    return (characters);
}

static char *convert_int(int number, int is_negative)
{
    int length = itoa_length(number);
    char *result = static_cast<char *>(cma_malloc(static_cast<ft_size_t>(length + 1 + is_negative)));
    if (result == ft_nullptr)
        return (ft_nullptr);
    unsigned int absolute_value = (number < 0) ? -static_cast<unsigned int>(number) : static_cast<unsigned int>(number);
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

char *adv_itoa(int number)
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
    int is_negative = (number < 0) ? 1 : 0;
    return (convert_int(number, is_negative));
}
