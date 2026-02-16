#include "../Basic/config.hpp"
#include "file_utils.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include <cstdio>

char *ft_fgets(char *string, int size, FILE *stream)
{
    if (string == ft_nullptr || stream == ft_nullptr || size <= 0)
        return (ft_nullptr);
    return (std::fgets(string, size, stream));
}
