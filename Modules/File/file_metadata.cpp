#include "file_utils.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

static int32_t file_metadata_compare_permissions(const char *left_path,
    const char *right_path, uint32_t *difference_value)
{
    mode_t left_permissions;
    mode_t right_permissions;
    int32_t error_code;

    error_code = file_get_permissions(left_path, &left_permissions);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = file_get_permissions(right_path, &right_permissions);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    if (left_permissions != right_permissions)
        *difference_value = *difference_value | FILE_METADATA_DIFF_PERMISSIONS;
    return (FT_ERR_SUCCESS);
}

int32_t file_metadata_diff(const char *left_path, const char *right_path,
    uint32_t *difference_out)
{
    file_type left_type;
    file_type right_type;
    ft_size_t left_size;
    ft_size_t right_size;
    uint32_t difference_value;
    int32_t error_code;

    if (left_path == ft_nullptr || right_path == ft_nullptr
        || difference_out == ft_nullptr)
        return (FT_ERR_INVALID_POINTER);
    difference_value = FILE_METADATA_DIFF_NONE;
    left_type = file_get_type(left_path);
    right_type = file_get_type(right_path);
    if (left_type == FILE_TYPE_UNKNOWN || right_type == FILE_TYPE_UNKNOWN)
        return (FT_ERR_IO);
    if (left_type == FILE_TYPE_MISSING || right_type == FILE_TYPE_MISSING)
    {
        if (left_type != right_type)
            difference_value = difference_value | FILE_METADATA_DIFF_MISSING;
        *difference_out = difference_value;
        return (FT_ERR_SUCCESS);
    }
    if (left_type != right_type)
        difference_value = difference_value | FILE_METADATA_DIFF_TYPE;
    if (left_type == FILE_TYPE_REGULAR && right_type == FILE_TYPE_REGULAR)
    {
        error_code = file_get_size(left_path, &left_size);
        if (error_code != FT_ERR_SUCCESS)
            return (error_code);
        error_code = file_get_size(right_path, &right_size);
        if (error_code != FT_ERR_SUCCESS)
            return (error_code);
        if (left_size != right_size)
            difference_value = difference_value | FILE_METADATA_DIFF_SIZE;
    }
    error_code = file_metadata_compare_permissions(left_path, right_path,
            &difference_value);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    *difference_out = difference_value;
    return (FT_ERR_SUCCESS);
}
