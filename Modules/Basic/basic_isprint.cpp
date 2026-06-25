#include "basic.hpp"

int32_t ft_isprint(int32_t character)
{
    if (character >= 32 && character <= 126)
        return (1);
    return (0);
}
