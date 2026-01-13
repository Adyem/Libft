#include "open_dir.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"

int file_create_directory(const char *path, mode_t mode)
{
    int result;
    int error_code;

    result = cmp_file_create_directory(path, mode, &error_code);
    if (result != 0 && error_code == FT_ERR_SUCCESSS)
        error_code = FT_ERR_INTERNAL;
    if (result == 0)
        error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (result);
}
