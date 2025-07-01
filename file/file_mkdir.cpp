#include "open_dir.hpp"

#ifdef _WIN32
# include <windows.h>
# include <sys/stat.h>
# include <sys/types.h>

int file_create_directory(const char* path, mode_t mode)
{
    (void)mode;
    if (CreateDirectoryA(path, NULL))
        return (0);
    return (-1);
}

#else
# include <sys/stat.h>
# include <sys/types.h>

int file_create_directory(const char* path, mode_t mode)
{
    if (mkdir(path, mode) == 0)
        return (0);
    return (-1);
}
#endif
