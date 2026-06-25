#include "open_dir.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

int32_t file_create_directory(const char *path, mode_t mode)
{
    int32_t result;
    int32_t create_error_code;

    result = cmp_file_create_directory(path, mode, &create_error_code);
    if (result != FT_ERR_SUCCESS && create_error_code == FT_ERR_SUCCESS)
        return (FT_ERR_IO);
    if (result != FT_ERR_SUCCESS)
        return (create_error_code);
    return (FT_ERR_SUCCESS);
}
