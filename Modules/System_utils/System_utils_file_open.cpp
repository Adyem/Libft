#include "system_utils.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"
#include <cerrno>

int32_t su_open(const char *path_name)
{
    int32_t result;

    result = cmp_open(path_name);
    if (result < 0)
    {
        if (cmp_map_system_error_to_ft(errno) == FT_ERR_SUCCESS)
            return (-1);
    }
    return (result);
}

int32_t su_open(const char *path_name, int32_t flags)
{
    int32_t result;

    result = cmp_open(path_name, flags);
    if (result < 0)
    {
        if (cmp_map_system_error_to_ft(errno) == FT_ERR_SUCCESS)
            return (-1);
    }
    return (result);
}

int32_t su_open(const char *path_name, int32_t flags, mode_t mode)
{
    int32_t result;

    result = cmp_open(path_name, flags, mode);
    if (result < 0)
    {
        if (cmp_map_system_error_to_ft(errno) == FT_ERR_SUCCESS)
            return (-1);
    }
    return (result);
}
