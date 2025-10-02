#include "libft.hpp"

int ft_isspace(int character)
{
    ft_errno = ER_SUCCESS;
    return (character == ' ' || character == '\f' || character == '\n' ||
            character == '\r' || character == '\t' || character == '\v');
}
