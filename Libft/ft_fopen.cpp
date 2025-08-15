#include "libft.hpp"
#include "../CPP_class/nullptr.hpp"
#include <cstdio>

FILE *ft_fopen(const char *filename, const char *mode)
{
    if (filename == ft_nullptr || mode == ft_nullptr)
        return (ft_nullptr);
    return (std::fopen(filename, mode));
}

