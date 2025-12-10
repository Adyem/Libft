#include "errno.hpp"
#include "../Printf/printf.hpp"

void    ft_perror(const char *error_msg)
{
    int saved_errno;

    saved_errno = ft_errno;
    if (!error_msg)
        pf_printf_fd(2, "%s\n", ft_strerror(saved_errno));
    else
        pf_printf_fd(2, "%s: %s\n", error_msg, ft_strerror(saved_errno));
    ft_errno = FT_ER_SUCCESSS;
    return ;
}
