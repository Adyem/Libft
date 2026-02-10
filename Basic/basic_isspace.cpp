#include "basic.hpp"

int32_t ft_isspace(int32_t character)
{
    if (character == ' ' || character == '\f' || character == '\n' ||
            character == '\r' || character == '\t' || character == '\v')
        return (1);
    return (0);
}
