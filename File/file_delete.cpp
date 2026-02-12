#include "file_utils.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"

int file_delete(const char *path)
{
    int result;
    int error_code;

    result = cmp_file_delete(path, &error_code);
    if (result != 0 && error_code == FT_ERR_SUCCESS)
        error_code = FT_ERR_INTERNAL;
    if (result == 0)
        error_code = FT_ERR_SUCCESS;
    ft_global_error_stack_push(error_code);
    return (result);
}
