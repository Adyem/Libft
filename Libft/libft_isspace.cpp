#include "libft.hpp"

int ft_isspace(int character)
{
    int result = 0;
    int error_code;

    if (character == ' ' || character == '\f' || character == '\n' ||
        character == '\r' || character == '\t' || character == '\v')
        result = 1;
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (result);
}
