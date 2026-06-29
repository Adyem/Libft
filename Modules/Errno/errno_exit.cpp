#include "errno_internal.hpp"
#include "errno.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include <cerrno>
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

void    su_exit(int32_t exit_code);

void    ft_exit(const char *error_msg, int32_t exit_code)
{
    int32_t error_code;

    if (errno == 0)
        error_code = FT_ERR_SUCCESS;
    else
        error_code = cmp_map_system_error_to_ft(errno);
    if (error_msg != ft_nullptr && error_code != FT_ERR_SUCCESS)
    {
        if (errno_write_stderr(error_msg) != FT_ERR_SUCCESS)
        {
            su_exit(exit_code);
        }
        if (errno_write_stderr(": ") != FT_ERR_SUCCESS)
        {
            su_exit(exit_code);
        }
        if (errno_write_stderr(ft_strerror(error_code)) != FT_ERR_SUCCESS)
        {
            su_exit(exit_code);
        }
        if (errno_write_stderr("\n") != FT_ERR_SUCCESS)
        {
            su_exit(exit_code);
        }
    }
    else if (error_msg != ft_nullptr)
    {
        if (errno_write_stderr(error_msg) != FT_ERR_SUCCESS)
        {
            su_exit(exit_code);
        }
        if (errno_write_stderr("\n") != FT_ERR_SUCCESS)
        {
            su_exit(exit_code);
        }
    }
    else if (error_code != FT_ERR_SUCCESS)
    {
        if (errno_write_stderr(ft_strerror(error_code)) != FT_ERR_SUCCESS)
        {
            su_exit(exit_code);
        }
        if (errno_write_stderr("\n") != FT_ERR_SUCCESS)
        {
            su_exit(exit_code);
        }
    }
    su_exit(exit_code);
    return ;
}
