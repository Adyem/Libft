#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/class_nullptr.hpp"
#include "open_dir.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

file_dir *file_opendir(const char *directory_path)
{
    file_dir *directory_stream;
    int32_t open_error_code;

    directory_stream = cmp_dir_open(directory_path, &open_error_code);
    if (directory_stream == ft_nullptr && open_error_code == FT_ERR_SUCCESS)
        return (ft_nullptr);
    return (directory_stream);
}

file_dirent *file_readdir(file_dir *directory_stream)
{
    file_dirent *directory_entry;
    int32_t read_error_code;

    if (!directory_stream)
        return (ft_nullptr);
    directory_entry = cmp_dir_read(directory_stream, &read_error_code);
    return (directory_entry);
}

int32_t file_closedir(file_dir *directory_stream)
{
    int32_t close_result;
    int32_t close_error_code;

    if (!directory_stream)
        return (FT_ERR_INVALID_ARGUMENT);
    close_result = cmp_dir_close(directory_stream, &close_error_code);
    if (close_result != FT_ERR_SUCCESS && close_error_code == FT_ERR_SUCCESS)
        return (FT_ERR_IO);
    if (close_result != FT_ERR_SUCCESS)
        return (close_error_code);
    return (FT_ERR_SUCCESS);
}
