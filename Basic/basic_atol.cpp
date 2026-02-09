#include "basic.hpp"
#include "../CPP_class/class_nullptr.hpp"

int64_t ft_atol(const char *string)
{
    if (string == ft_nullptr)
        return (0);
    const char *cursor = string;
    while (*cursor == ' ' || (*cursor >= '\t' && *cursor <= '\r'))
        cursor++;
    int sign = 1;
    if (*cursor == '+' || *cursor == '-')
    {
        if (*cursor == '-')
            sign = -1;
        cursor++;
    }
    if (*cursor < '0' || *cursor > '9')
        return (0);
    int64_t value = 0;
    while (*cursor >= '0' && *cursor <= '9')
    {
        int digit = *cursor - '0';
        value = value * 10 + digit;
        cursor++;
    }
    if (*cursor != '\0')
    {
        if (sign == -1)
            return (-value);
        return (value);
    }
    if (sign == -1)
        return (-value);
    return (value);
}
