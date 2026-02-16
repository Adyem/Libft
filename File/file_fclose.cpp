#include "../Basic/config.hpp"
#include "file_utils.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include <cstdio>

int ft_fclose(FILE *stream)
{
    if (stream == ft_nullptr)
        return (EOF);
    if (std::fclose(stream) != 0)
        return (EOF);
    return (FT_SUCCESS);
}
