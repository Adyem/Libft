#include "libft.hpp"

int ft_isupper(int character)
{
    int error_code;

    if (character >= 'A' && character <= 'Z')
    {
        error_code = FT_ERR_SUCCESSS;
        ft_global_error_stack_push(error_code);
        return (1);
    }
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (0);
}
