#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

void ft_to_lower(char *string)
{
    ft_errno = ER_SUCCESS;
    if (string == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return ;
    }
    while (*string != '\0')
    {
        if (*string >= 'A' && *string <= 'Z')
            *string += 32;
        string++;
    }
    return ;
}
