#include "libft.hpp"

int ft_isspace(int character)
{
    ft_errno = FT_ERR_SUCCESSS;
    return (character == ' ' || character == '\f' || character == '\n' ||
            character == '\r' || character == '\t' || character == '\v');
}
