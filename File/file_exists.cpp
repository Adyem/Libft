#include "file_utils.hpp"
#include "../Compatebility/compatebility_internal.hpp"

int file_exists(const char *path)
{
    int32_t status;
    int32_t exists_value;
    int32_t error_code;

    exists_value = 0;
    status = cmp_file_exists(path, &exists_value, &error_code);
    if (status != FT_ERR_SUCCESS)
        return (0);
    return (exists_value);
}
