#include "libft.hpp"

int ft_islower(int character)
{
    if (character >= 'a' && character <= 'z')
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (1);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}
