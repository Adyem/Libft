#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

void ft_to_upper(char *string)
{
    if (string == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    while (*string != '\0')
    {
        if (*string >= 'a' && *string <= 'z')
            *string -= 32;
        string++;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}
