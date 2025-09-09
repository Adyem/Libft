#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include <cstdio>

int ft_fseek(FILE *stream, long offset, int origin)
{
    if (stream == ft_nullptr)
        return (-1);
    return (std::fseek(stream, offset, origin));
}

