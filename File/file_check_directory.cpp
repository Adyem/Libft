#include <cstdlib>
#include "../CPP_class/class_string.hpp"
#include "../Errno/errno.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "open_dir.hpp"

static ft_string normalize_path(ft_string path)
{
    char *data = path.print();
    cmp_normalize_slashes(data);
    return (path);
}

int file_dir_exists(const char *rel_path)
{
    ft_string path = normalize_path(rel_path);
    if (path.get_error())
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    return (cmp_directory_exists(path.c_str()));
}
