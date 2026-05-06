#include "basic.hpp"

void ft_to_lower(char *string)
{
    if (!string)
        return ;
    while (*string != '\0')
    {
        if (*string >= 'A' && *string <= 'Z')
            *string += 32;
        string++;
    }
    return ;
}
