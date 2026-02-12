#include "../Basic/config.hpp"
#include "file_utils.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"
#include <cerrno>
#include <cstdio>

int ft_fclose(FILE *stream)
{
    int close_result;
    int error_code;

    if (stream == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_HANDLE);
        return (EOF);
    }
    close_result = std::fclose(stream);
    if (close_result != 0)
    {
        int close_error;

        close_error = errno;
        if (close_error != 0)
            error_code = cmp_map_system_error_to_ft(close_error);
        else
            error_code = FT_ERR_INVALID_HANDLE;
        ft_global_error_stack_push(error_code);
        return (EOF);
    }
    error_code = FT_ERR_SUCCESS;
    ft_global_error_stack_push(error_code);
    return (FT_SUCCESS);
}
