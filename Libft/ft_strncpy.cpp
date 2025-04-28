#include "libft.hpp"
#include "../CPP_class/nullptr.hpp"

char *ft_strncpy(char *destination, const char *source, size_t number_of_characters)
{
    if (destination == ft_nullptr || source == ft_nullptr)
        return (ft_nullptr);
    size_t index = 0;
    while (index < number_of_characters && source[index] != '\0')
    {
        destination[index] = source[index];
        index++;
    }
    while (index < number_of_characters)
    {
        destination[index] = '\0';
        index++;
    }
    return (destination);
}
