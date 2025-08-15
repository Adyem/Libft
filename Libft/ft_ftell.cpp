#include "libft.hpp"
#include "../CPP_class/nullptr.hpp"
#include <cstdio>

long ft_ftell(FILE *stream)
{
    if (stream == ft_nullptr)
        return (-1L);
    return (std::ftell(stream));
}

