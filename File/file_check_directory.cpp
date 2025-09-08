#include <cstdlib>
#include "../CPP_class/string_class.hpp"
#include "../Errno/errno.hpp"
#include "file_open_dir.hpp"

#if defined(_WIN32) || defined(_WIN64)
  #include <windows.h>
  #define PATH_SEP '\\'
#else
  #include <sys/stat.h>
  #include <unistd.h>
  #define PATH_SEP '/'
#endif

[[maybe_unused]] static inline void normalize_slashes(char *data)
{
#if defined(_WIN32) || defined(_WIN64)
    if (!data)
        return ;
    size_t index = 0;
    while (data[index] != '\0')
    {
        if (data[index] == '/')
            data[index] = PATH_SEP;
        ++index;
    }
#else
    (void)data;
#endif
    return ;
}

static inline int file_dir_exists_platform(const char *path)
{
#if defined(_WIN32) || defined(_WIN64)
    DWORD attr = GetFileAttributesA(path);
    if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY))
        return (0);
#else
    struct stat stat_buffer;
    if (stat(path, &stat_buffer) == 0 && S_ISDIR(stat_buffer.st_mode))
        return (0);
#endif
    return (1);
}

static ft_string normalize_path(ft_string path)
{
#if defined(_WIN32) || defined(_WIN64)
    char* data = path.print();
    normalize_slashes(data);
#else
    (void)path;
#endif
    return (path);
}

int file_dir_exists(const char *rel_path)
{
    ft_string path = normalize_path(rel_path);
    if (path.get_error())
    {
        ft_errno = CHECK_DIR_FAIL;
        return (-1);
    }
    return (file_dir_exists_platform(path.c_str()));
}
