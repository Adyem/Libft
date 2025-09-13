#include "open_dir.hpp"
#include "../Compatebility/compatebility_internal.hpp"

int file_create_directory(const char *path, mode_t mode)
{
    return (cmp_file_create_directory(path, mode));
}

