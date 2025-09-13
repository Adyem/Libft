#include "file_utils.hpp"
#include "../Compatebility/compatebility_internal.hpp"

int file_move(const char *source_path, const char *destination_path)
{
    return (cmp_file_move(source_path, destination_path));
}

