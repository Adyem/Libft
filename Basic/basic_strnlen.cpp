#include "basic.hpp"

ft_size_t ft_strnlen(const char *string, ft_size_t maximum_length)
{
    if (!string)
        return (0);
    ft_size_t index = 0;
    while (index < maximum_length && string[index] != '\0')
        ++index;
    return (index);
}
