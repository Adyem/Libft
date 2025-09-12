#include "file_utils.hpp"

#ifdef _WIN32
# include <stdio.h>

int file_delete(const char *path)
{
    if (remove(path) == 0)
        return (0);
    return (-1);
}

#else
# include <unistd.h>

int file_delete(const char *path)
{
    if (unlink(path) == 0)
        return (0);
    return (-1);
}
#endif

