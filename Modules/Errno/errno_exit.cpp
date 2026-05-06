#include "errno.hpp"
#include "../Printf/printf.hpp"
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
    if (error_msg && error_code != FT_ERR_SUCCESS)
        pf_printf_fd(2, "%s: %s\n", error_msg, ft_strerror(error_code));
    else if (error_msg)
        pf_printf_fd(2, "%s\n", error_msg);
    else if (error_code != FT_ERR_SUCCESS)
        pf_printf_fd(2, "%s\n", ft_strerror(error_code));
    std::_Exit(exit_code);
    return ;
}
