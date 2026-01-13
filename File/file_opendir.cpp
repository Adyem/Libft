#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "open_dir.hpp"

file_dir *file_opendir(const char *directory_path)
{
    file_dir *directory_stream;
    int error_code;

    directory_stream = cmp_dir_open(directory_path, &error_code);
    if (directory_stream == ft_nullptr && error_code == FT_ERR_SUCCESSS)
        error_code = FT_ERR_INTERNAL;
    ft_global_error_stack_push(error_code);
    return (directory_stream);
}

file_dirent *file_readdir(file_dir *directory_stream)
{
    file_dirent *directory_entry;
    int error_code;

    if (!directory_stream)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    directory_entry = cmp_dir_read(directory_stream, &error_code);
    ft_global_error_stack_push(error_code);
    return (directory_entry);
}

int file_closedir(file_dir *directory_stream)
{
    int close_result;
    int error_code;

    if (!directory_stream)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    close_result = cmp_dir_close(directory_stream, &error_code);
    if (close_result != 0 && error_code == FT_ERR_SUCCESSS)
        error_code = FT_ERR_INTERNAL;
    if (close_result == 0)
        error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (close_result);
}
