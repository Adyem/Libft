#include "file_utils.hpp"

#ifdef _WIN32
# include <windows.h>

int file_move(const char *source_path, const char *destination_path)
{
    if (MoveFileA(source_path, destination_path))
        return (0);
    return (-1);
}

#else
# include <stdio.h>

int file_move(const char *source_path, const char *destination_path)
{
    if (rename(source_path, destination_path) == 0)
        return (0);
    return (-1);
}
#endif
