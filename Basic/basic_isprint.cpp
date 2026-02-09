#include "basic.hpp"

int ft_isprint(int character)
{
    if (character >= 32 && character <= 126)
        return (1);
    return (0);
}
