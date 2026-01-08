#include "file_utils.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"

int file_exists(const char *path)
{
    int result;
    int error_code;

    result = cmp_file_exists(path);
    error_code = ft_errno;
    if (result == 0 && error_code == FT_ERR_SUCCESSS)
        error_code = FT_ERR_INTERNAL;
    ft_global_error_stack_push(error_code);
    return (result);
}
