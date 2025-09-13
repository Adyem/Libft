#include "file_utils.hpp"
#include "../Compatebility/compatebility_internal.hpp"

int file_copy(const char *source_path, const char *destination_path)
{
    return (cmp_file_copy(source_path, destination_path));
}

