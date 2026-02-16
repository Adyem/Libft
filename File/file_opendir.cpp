#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "open_dir.hpp"

file_dir *file_opendir(const char *directory_path)
{
    file_dir *directory_stream;
    int open_error_code;

    directory_stream = cmp_dir_open(directory_path, &open_error_code);
    if (directory_stream == ft_nullptr && open_error_code == FT_ERR_SUCCESS)
        return (ft_nullptr);
    return (directory_stream);
}

file_dirent *file_readdir(file_dir *directory_stream)
{
    file_dirent *directory_entry;
    int read_error_code;

    if (!directory_stream)
        return (ft_nullptr);
    directory_entry = cmp_dir_read(directory_stream, &read_error_code);
    return (directory_entry);
}

int file_closedir(file_dir *directory_stream)
{
    int close_result;
    int close_error_code;

    if (!directory_stream)
        return (-1);
    close_result = cmp_dir_close(directory_stream, &close_error_code);
    if (close_result != 0 && close_error_code == FT_ERR_SUCCESS)
        return (-1);
    return (close_result);
}
