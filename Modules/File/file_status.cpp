#include "file_utils.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

file_type file_get_type(const char *path) noexcept
{
    file_type type_value;
    int32_t error_code;

    type_value = FILE_TYPE_UNKNOWN;
    if (cmp_file_get_type(path, &type_value, &error_code) != FT_ERR_SUCCESS)
        return (FILE_TYPE_UNKNOWN);
    return (type_value);
}

int32_t file_get_size(const char *path, ft_size_t *size)
{
    int32_t result;
    int32_t error_code;

    result = cmp_file_get_size(path, size, &error_code);
    if (result != FT_ERR_SUCCESS && error_code == FT_ERR_SUCCESS)
        return (FT_ERR_IO);
    if (result != FT_ERR_SUCCESS)
        return (error_code);
    return (FT_ERR_SUCCESS);
}

int32_t file_get_permissions(const char *path, mode_t *mode)
{
    int32_t result;
    int32_t error_code;

    result = cmp_file_get_permissions(path, mode, &error_code);
    if (result != FT_ERR_SUCCESS && error_code == FT_ERR_SUCCESS)
        return (FT_ERR_IO);
    if (result != FT_ERR_SUCCESS)
        return (error_code);
    return (FT_ERR_SUCCESS);
}
