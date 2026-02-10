#include "basic.hpp"

int    ft_isdigit(int32_t character)
{
    if (character >= '0' && character <= '9')
        return (1);
    return (0);
}
