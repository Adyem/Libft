#include <cstdlib>
#include "../CPP_class/class_string.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "open_dir.hpp"

static ft_string normalize_path(ft_string path)
{
    char *data = path.print();

    if (data != ft_nullptr)
        cmp_normalize_slashes(data);
    return (path);
}

int file_dir_exists(const char *rel_path)
{
    ft_string path = normalize_path(rel_path);
    int error_code;
    int exists_value;
    int status;

    if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
        return (-1);
    exists_value = 0;
    status = cmp_directory_exists(path.c_str(), &exists_value, &error_code);
    if (status != FT_ERR_SUCCESS)
        return (-1);
    return (exists_value);
}
