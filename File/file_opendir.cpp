#include "../Libft/libft.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "open_dir.hpp"

file_dir *file_opendir(const char *directory_path)
{
    return (cmp_dir_open(directory_path));
}

file_dirent *file_readdir(file_dir *directory_stream)
{
    file_dirent *directory_entry;

    if (!directory_stream)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    directory_entry = cmp_dir_read(directory_stream);
    if (directory_entry != ft_nullptr)
        ft_errno = ER_SUCCESS;
    return (directory_entry);
}

int file_closedir(file_dir *directory_stream)
{
    int close_result;

    if (!directory_stream)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    close_result = cmp_dir_close(directory_stream);
    if (close_result == 0)
        ft_errno = ER_SUCCESS;
    return (close_result);
}

