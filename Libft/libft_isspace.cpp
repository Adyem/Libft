#include "libft.hpp"

int ft_isspace(int character)
{
    int result = 0;

    if (character == ' ' || character == '\f' || character == '\n' ||
        character == '\r' || character == '\t' || character == '\v')
        result = 1;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}
