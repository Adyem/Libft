#include "libft.hpp"
#include "../CPP_class/cpp_class_nullptr.hpp"
#include <cstdio>

size_t ft_fwrite(const void *ptr, size_t size, size_t count, FILE *stream)
{
    if (ptr == ft_nullptr || stream == ft_nullptr)
        return (0);
    return (std::fwrite(ptr, size, count, stream));
}

