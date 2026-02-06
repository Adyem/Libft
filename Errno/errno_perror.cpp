#include "errno.hpp"
#include "../Printf/printf.hpp"

void    ft_perror(const char *error_msg)
{
    int error_code;

    error_code = ft_global_error_stack_peek_last_error();
    if (!error_msg)
        pf_printf_fd(2, "%s\n", ft_strerror(error_code));
    else
        pf_printf_fd(2, "%s: %s\n", error_msg, ft_strerror(error_code));
    return ;
}
