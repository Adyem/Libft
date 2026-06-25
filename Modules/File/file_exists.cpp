#include "file_utils.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

int32_t file_exists(const char *path)
{
    int32_t status;
    int32_t exists_value;
    int32_t error_code;

    exists_value = 0;
    status = cmp_file_exists(path, &exists_value, &error_code);
    if (status != FT_ERR_SUCCESS)
    {
        if (error_code == FT_ERR_INVALID_ARGUMENT)
            return (error_code);
        return (0);
    }
    if (exists_value != FT_ERR_SUCCESS)
        return (1);
    return (0);
}
