#include "errno_internal.hpp"
#include "errno.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include <cstdlib>
#include <cerrno>

void    ft_exit(const char *error_msg, int32_t exit_code)
{
    int32_t error_code;

    if (errno == 0)
        error_code = FT_ERR_SUCCESS;
    else
        error_code = cmp_map_system_error_to_ft(errno);
    if (error_msg != ft_nullptr && error_code != FT_ERR_SUCCESS)
    {
        errno_write_stderr(error_msg);
        errno_write_stderr(": ");
        errno_write_stderr(ft_strerror(error_code));
        errno_write_stderr("\n");
    }
    else if (error_msg != ft_nullptr)
    {
        errno_write_stderr(error_msg);
        errno_write_stderr("\n");
    }
    else if (error_code != FT_ERR_SUCCESS)
    {
        errno_write_stderr(ft_strerror(error_code));
        errno_write_stderr("\n");
    }
    std::_Exit(exit_code);
    return ;
}
