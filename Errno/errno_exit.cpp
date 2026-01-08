#include "errno.hpp"
#include "../Printf/printf.hpp"
#include <cstdlib>

void    ft_exit(const char *error_msg, int exit_code)
{
    int error_code;

    error_code = ft_global_error_stack_last_error();
    if (error_msg && error_code != FT_ERR_SUCCESSS)
        pf_printf_fd(2, "%s: %s\n", error_msg, ft_strerror(error_code));
    else if (error_msg)
        pf_printf_fd(2, "%s\n", error_msg);
    else if (error_code != FT_ERR_SUCCESSS)
        pf_printf_fd(2, "%s\n", ft_strerror(error_code));
    std::_Exit(exit_code);
    return ;
}
