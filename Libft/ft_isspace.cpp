#include "libft.hpp"

int ft_isspace(int character)
{
    return (character == ' ' || character == '\f' || character == '\n' ||
            character == '\r' || character == '\t' || character == '\v');
}
