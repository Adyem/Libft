#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include <cstdio>

size_t ft_fread(void *ptr, size_t size, size_t count, FILE *stream)
{
    if (ptr == ft_nullptr || stream == ft_nullptr)
        return (0);
    return (std::fread(ptr, size, count, stream));
}

