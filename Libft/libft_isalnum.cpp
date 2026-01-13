#include "libft.hpp"

int ft_isalnum(int character)
{
    int error_code;

    if (ft_isdigit(character) || ft_isalpha(character))
    {
        error_code = FT_ERR_SUCCESSS;
        ft_global_error_stack_push(error_code);
        return (1);
    }
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (0);
}
