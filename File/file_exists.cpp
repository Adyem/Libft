#include "file_utils.hpp"

#ifdef _WIN32
# include <windows.h>

int file_exists(const char *path)
{
    DWORD file_attributes = GetFileAttributesA(path);
    if (file_attributes != INVALID_FILE_ATTRIBUTES && !(file_attributes & FILE_ATTRIBUTE_DIRECTORY))
        return (1);
    return (0);
}

#else
# include <sys/stat.h>

int file_exists(const char *path)
{
    struct stat stat_buffer;
    if (stat(path, &stat_buffer) == 0 && S_ISREG(stat_buffer.st_mode))
        return (1);
    return (0);
}
#endif

