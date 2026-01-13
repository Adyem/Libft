#include "libft.hpp"

int ft_isprint(int character)
{
    int error_code;

    if (character >= 32 && character <= 126)
    {
        error_code = FT_ERR_SUCCESSS;
        ft_global_error_stack_push(error_code);
        return (1);
    }
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (0);
}
