#include "open_dir.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"

int file_create_directory(const char *path, mode_t mode)
{
    int result;
    int create_error_code;

    result = cmp_file_create_directory(path, mode, &create_error_code);
    if (result != 0 && create_error_code == FT_ERR_SUCCESS)
        return (-1);
    return (result);
}
