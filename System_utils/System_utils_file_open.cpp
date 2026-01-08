#include "system_utils.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"

int su_open(const char *path_name)
{
    int result;
    int error_code;

    result = cmp_open(path_name);
    error_code = ft_errno;
    if (result < 0 && error_code == FT_ERR_SUCCESSS)
        error_code = FT_ERR_INTERNAL;
    if (result >= 0)
        error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (result);
}

int su_open(const char *path_name, int flags)
{
    int result;
    int error_code;

    result = cmp_open(path_name, flags);
    error_code = ft_errno;
    if (result < 0 && error_code == FT_ERR_SUCCESSS)
        error_code = FT_ERR_INTERNAL;
    if (result >= 0)
        error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (result);
}

int su_open(const char *path_name, int flags, mode_t mode)
{
    int result;
    int error_code;

    result = cmp_open(path_name, flags, mode);
    error_code = ft_errno;
    if (result < 0 && error_code == FT_ERR_SUCCESSS)
        error_code = FT_ERR_INTERNAL;
    if (result >= 0)
        error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (result);
}
