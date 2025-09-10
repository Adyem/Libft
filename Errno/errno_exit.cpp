#include "errno.hpp"
#include "../Printf/printf.hpp"
#include <cstdlib>

void    ft_exit(const char *error_msg, int exit_code)
{
    if (error_msg && _error_code != ER_SUCCESS)
        pf_printf_fd(2, "%s: %s\n", error_msg, ft_strerror(_error_code));
    else if (error_msg)
        pf_printf_fd(2, "%s\n", error_msg);
    else if (_error_code != ER_SUCCESS)
        pf_printf_fd(2, "%s\n", ft_strerror(_error_code));
    std::exit(exit_code);
}

