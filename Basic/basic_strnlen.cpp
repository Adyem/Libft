#include "basic.hpp"

size_t ft_strnlen(const char *string, size_t maximum_length)
{
    if (!string)
        return (0);
    size_t index = 0;
    while (index < maximum_length && string[index] != '\0')
        ++index;
    return (index);
}
