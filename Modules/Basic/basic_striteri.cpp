#include "basic.hpp"

void ft_striteri(char *string, void (*function)(uint32_t, char *))
{
    if (!string || !function)
        return ;
    uint32_t index = 0;
    while (string[index] != '\0')
    {
        function(index, &string[index]);
        ++index;
    }
    return ;
}
