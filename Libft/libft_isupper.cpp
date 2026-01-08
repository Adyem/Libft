#include "libft.hpp"

int ft_isupper(int character)
{
    if (character >= 'A' && character <= 'Z')
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (1);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}
