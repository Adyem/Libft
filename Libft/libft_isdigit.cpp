#include "libft.hpp"

int    ft_isdigit(int character)
{
    if (character >= '0' && character <= '9')
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (1);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}
