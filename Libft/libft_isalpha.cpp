#include "libft.hpp"

int ft_isalpha(int character)
{
    int error_code;

    if ((character >= 'a' && character <= 'z') ||
        (character >= 'A' && character <= 'Z'))
    {
        error_code = FT_ERR_SUCCESSS;
        ft_global_error_stack_push(error_code);
        return (1);
    }
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (0);
}
