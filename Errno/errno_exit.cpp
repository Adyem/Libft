#include "errno.hpp"
#include "../Printf/printf.hpp"
#include <cstdlib>

void    ft_exit(const char *error_msg, int exit_code)
{
    if (error_msg && ft_errno != ER_SUCCESS)
        pf_printf_fd(2, "%s: %s\n", error_msg, ft_strerror(ft_errno));
    else if (error_msg)
        pf_printf_fd(2, "%s\n", error_msg);
    else if (ft_errno != ER_SUCCESS)
        pf_printf_fd(2, "%s\n", ft_strerror(ft_errno));
    std::exit(exit_code);
}

