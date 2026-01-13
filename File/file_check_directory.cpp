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
    int error_code;
    int result;

    error_code = path.get_error();
    if (error_code != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    result = cmp_directory_exists(path.c_str(), &error_code);
    ft_global_error_stack_push(error_code);
    return (result);
}
