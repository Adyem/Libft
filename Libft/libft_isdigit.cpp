#include "libft.hpp"

int    ft_isdigit(int character)
{
    int error_code;

    if (character >= '0' && character <= '9')
    {
        error_code = FT_ERR_SUCCESSS;
        ft_global_error_stack_push(error_code);
        return (1);
    }
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (0);
}
