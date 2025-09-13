#include "file_utils.hpp"
#include "../Compatebility/compatebility_internal.hpp"

int file_delete(const char *path)
{
    return (cmp_file_delete(path));
}

