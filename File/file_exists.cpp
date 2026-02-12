#include "file_utils.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"

int file_exists(const char *path)
{
    int32_t status;
    int32_t exists_value;
    int32_t error_code;

    exists_value = 0;
    status = cmp_file_exists(path, &exists_value, &error_code);
    if (status != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(error_code);
        return (0);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (exists_value);
}
