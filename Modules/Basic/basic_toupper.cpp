#include "basic.hpp"

void ft_to_upper(char *string)
{
    if (!string)
        return ;
    while (*string != '\0')
    {
        if (*string >= 'a' && *string <= 'z')
            *string -= 32;
        string++;
    }
    return ;
}
