#include "errno.hpp"
#include "../Printf/printf.hpp"

void    ft_perror(const char *error_msg, int32_t error_code)
{
    if (!error_msg)
        pf_printf_fd(2, "%s\n", ft_strerror(error_code));
    else
        pf_printf_fd(2, "%s: %s\n", error_msg, ft_strerror(error_code));
    return ;
}
