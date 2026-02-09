#include "basic.hpp"

void ft_striteri(char *string, void (*function)(unsigned int, char *))
{
    if (!string || !function)
        return ;
    unsigned int index = 0;
    while (string[index] != '\0')
    {
        function(index, &string[index]);
        ++index;
    }
    return ;
}
