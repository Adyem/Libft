#include "../CPP_class/class_nullptr.hpp"
#include "../CPP_class/class_string.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"
#include "file_utils.hpp"

static int file_string_error(const ft_string &string_value) noexcept
{
    unsigned long long operation_id;
    int               error_code;

    operation_id = string_value.last_operation_id();
    if (operation_id == 0)
        return (FT_ERR_SUCCESS);
    error_code = string_value.pop_operation_error(operation_id);
    if (error_code != FT_ERR_SUCCESS)
        ft_global_error_stack_push_entry_with_id(error_code, operation_id);
    return (error_code);
}

ft_string file_path_normalize(const char *path)
{
    ft_string empty_result;
    int error_code;

    if (path == ft_nullptr)
    {
        error_code = file_string_error(empty_result);
        if (error_code != FT_ERR_SUCCESS)
            return (empty_result);
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (empty_result);
    }
    ft_string original(path);
    error_code = file_string_error(original);
    if (error_code != FT_ERR_SUCCESS)
        return (original);
    char *data = original.print();
    if (!data)
    {
        ft_global_error_stack_push(FT_ERR_INTERNAL);
        return (empty_result);
    }
    cmp_normalize_slashes(data);
    ft_string result;
    error_code = file_string_error(result);
    if (error_code != FT_ERR_SUCCESS)
        return (result);
    size_t index = 0;
    char path_sep = cmp_path_separator();
    while (data[index] != '\0')
    {
        if (data[index] == path_sep)
        {
            result.append(path_sep);
            while (data[index] == path_sep)
            {
                ++index;
            }
        }
        else
        {
            result.append(data[index]);
            ++index;
        }
    }
    error_code = file_string_error(result);
    if (error_code != FT_ERR_SUCCESS)
        return (result);
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (result);
}
