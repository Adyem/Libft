#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

void ft_to_upper(char *string)
{
    ft_errno = ER_SUCCESS;
    if (string == ft_nullptr)
    {
        ft_errno = FT_EINVAL;
        return ;
    }
    while (*string != '\0')
    {
        if (*string >= 'a' && *string <= 'z')
            *string -= 32;
        string++;
    }
    return ;
}
