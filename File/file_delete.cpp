#include "file_utils.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"

int file_delete(const char *path)
{
    int result;
    int delete_error_code;

    result = cmp_file_delete(path, &delete_error_code);
    if (result != 0 && delete_error_code == FT_ERR_SUCCESS)
        return (-1);
    return (result);
}
