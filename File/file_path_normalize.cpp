#include "../CPP_class/class_nullptr.hpp"
#include "../CPP_class/class_string.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"
#include "file_utils.hpp"

ft_string file_path_normalize(const char *path)
{
    ft_string empty_result;
    int error_code;

    if (path == ft_nullptr)
    {
        error_code = empty_result.get_error();
        if (error_code != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(error_code);
            return (empty_result);
        }
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (empty_result);
    }
    ft_string original(path);
    error_code = original.get_error();
    if (error_code != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(error_code);
        return (original);
    }
    char *data = original.print();
    if (!data)
    {
        ft_global_error_stack_push(FT_ERR_INTERNAL);
        return (empty_result);
    }
    cmp_normalize_slashes(data);
    ft_string result;
    error_code = result.get_error();
    if (error_code != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(error_code);
        return (result);
    }
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
    error_code = result.get_error();
    if (error_code != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(error_code);
        return (result);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}
