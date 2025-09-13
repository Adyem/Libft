#include "../Libft/libft.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "open_dir.hpp"

file_dir *file_opendir(const char *directory_path)
{
    return (cmp_dir_open(directory_path));
}

file_dirent *file_readdir(file_dir *directory_stream)
{
    if (!directory_stream)
        return (ft_nullptr);
    return (cmp_dir_read(directory_stream));
}

int file_closedir(file_dir *directory_stream)
{
    if (!directory_stream)
        return (-1);
    return (cmp_dir_close(directory_stream));
}

