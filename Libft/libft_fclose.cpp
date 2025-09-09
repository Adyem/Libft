#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include <cstdio>

int ft_fclose(FILE *stream)
{
    if (stream == ft_nullptr)
        return (EOF);
    return (std::fclose(stream));
}

