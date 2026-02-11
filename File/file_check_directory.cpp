#include <cstdlib>
#include "../CPP_class/class_string.hpp"
#include "../Errno/errno.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "open_dir.hpp"

static int file_string_error(const ft_string &string_value) noexcept
{
    unsigned long long operation_id;
    int               error_code;

    operation_id = string_value.last_operation_id();
    if (operation_id == 0)
        return (FT_ERR_SUCCESSS);
    error_code = string_value.pop_operation_error(operation_id);
    if (error_code != FT_ERR_SUCCESSS)
        ft_global_error_stack_push_entry_with_id(error_code, operation_id);
    return (error_code);
}

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
    int exists_value;
    int status;

    error_code = file_string_error(path);
    if (error_code != FT_ERR_SUCCESSS)
        return (-1);
    exists_value = 0;
    status = cmp_directory_exists(path.c_str(), &exists_value, &error_code);
    if (status != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    result = exists_value;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}
