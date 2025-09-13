#include "file_utils.hpp"
#include "../Compatebility/compatebility_internal.hpp"

int file_exists(const char *path)
{
    return (cmp_file_exists(path));
}

