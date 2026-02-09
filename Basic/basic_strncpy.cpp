#include <cstddef>
#include "basic.hpp"

char *ft_strncpy(char *destination, const char *source, size_t number_of_characters)
{
    if (!destination || !source)
        return (destination);
    size_t index = 0;
    while (index < number_of_characters && source[index] != 0)
    {
        destination[index] = source[index];
        ++index;
    }
    while (index < number_of_characters)
    {
        destination[index] = 0;
        ++index;
    }
    return (destination);
}
