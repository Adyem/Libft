#include "basic.hpp"
#include "../CPP_class/class_nullptr.hpp"

char *ft_strncpy(char *destination, const char *source, ft_size_t number_of_characters)
{
    if (number_of_characters == 0)
        return (destination);

    if (!destination || !source)
        return (ft_nullptr);
    ft_size_t index = 0;
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
