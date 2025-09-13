#include "system_utils.hpp"
#include "../Compatebility/compatebility_internal.hpp"

int su_open(const char *path_name)
{
    return (cmp_open(path_name));
}

int su_open(const char *path_name, int flags)
{
    return (cmp_open(path_name, flags));
}

int su_open(const char *path_name, int flags, mode_t mode)
{
    return (cmp_open(path_name, flags, mode));
}

