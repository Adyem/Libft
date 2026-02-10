#include "basic.hpp"

int ft_isalpha(int32_t character)
{
    if ((character >= 'a' && character <= 'z') ||
        (character >= 'A' && character <= 'Z'))
        return (1);
    return (0);
}
