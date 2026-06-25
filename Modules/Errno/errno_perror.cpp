#include "errno_internal.hpp"
#include "errno.hpp"
#include "../Basic/class_nullptr.hpp"

void    ft_perror(const char *error_msg, int32_t error_code)
{
    if (error_msg == ft_nullptr)
    {
        errno_write_stderr(ft_strerror(error_code));
        errno_write_stderr("\n");
        return ;
    }
    errno_write_stderr(error_msg);
    if (error_code == FT_ERR_SUCCESS)
    {
        errno_write_stderr("\n");
        return ;
    }
    errno_write_stderr(": ");
    errno_write_stderr(ft_strerror(error_code));
    errno_write_stderr("\n");
    return ;
}
