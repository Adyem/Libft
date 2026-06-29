#include "errno_internal.hpp"
#include "errno.hpp"
#include "../Basic/class_nullptr.hpp"

int32_t ft_perror(const char *error_msg, int32_t error_code)
{
    if (error_msg == ft_nullptr)
    {
        if (errno_write_stderr(ft_strerror(error_code)) != FT_ERR_SUCCESS)
            return (FT_ERR_SYSTEM);
        if (errno_write_stderr("\n") != FT_ERR_SUCCESS)
            return (FT_ERR_SYSTEM);
        return (FT_ERR_SUCCESS);
    }
    if (errno_write_stderr(error_msg) != FT_ERR_SUCCESS)
        return (FT_ERR_SYSTEM);
    if (error_code == FT_ERR_SUCCESS)
    {
        if (errno_write_stderr("\n") != FT_ERR_SUCCESS)
            return (FT_ERR_SYSTEM);
        return (FT_ERR_SUCCESS);
    }
    if (errno_write_stderr(": ") != FT_ERR_SUCCESS)
        return (FT_ERR_SYSTEM);
    if (errno_write_stderr(ft_strerror(error_code)) != FT_ERR_SUCCESS)
        return (FT_ERR_SYSTEM);
    if (errno_write_stderr("\n") != FT_ERR_SUCCESS)
        return (FT_ERR_SYSTEM);
    return (FT_ERR_SUCCESS);
}
