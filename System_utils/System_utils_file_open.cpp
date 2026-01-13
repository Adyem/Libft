#include "system_utils.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"
#include <cerrno>

int su_open(const char *path_name)
{
    int result;
    int error_code;

    result = cmp_open(path_name);
    if (result < 0)
    {
        error_code = cmp_map_system_error_to_ft(errno);
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_INTERNAL;
    }
    else
        error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (result);
}

int su_open(const char *path_name, int flags)
{
    int result;
    int error_code;

    result = cmp_open(path_name, flags);
    if (result < 0)
    {
        error_code = cmp_map_system_error_to_ft(errno);
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_INTERNAL;
    }
    else
        error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (result);
}

int su_open(const char *path_name, int flags, mode_t mode)
{
    int result;
    int error_code;

    result = cmp_open(path_name, flags, mode);
    if (result < 0)
    {
        error_code = cmp_map_system_error_to_ft(errno);
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_INTERNAL;
    }
    else
        error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (result);
}
