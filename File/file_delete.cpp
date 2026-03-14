#include "file_utils.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"

int32_t file_delete(const char *path)
{
    int32_t result;
    int32_t delete_error_code;

    result = cmp_file_delete(path, &delete_error_code);
    if (result != FT_ERR_SUCCESS && delete_error_code == FT_ERR_SUCCESS)
        return (FT_ERR_IO);
    if (result != FT_ERR_SUCCESS)
        return (delete_error_code);
    return (FT_ERR_SUCCESS);
}
