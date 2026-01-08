#include "libft.hpp"

int ft_isalnum(int character)
{
    if (ft_isdigit(character) || ft_isalpha(character))
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (1);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}
