#include "libft.hpp"

int ft_isprint(int character)
{
    if (character >= 32 && character <= 126)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (1);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}
