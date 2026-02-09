#include "basic.hpp"

int ft_isspace(int character)
{
    if (character == ' ' || character == '\f' || character == '\n' ||
            character == '\r' || character == '\t' || character == '\v')
        return (1);
    return (0);
}
