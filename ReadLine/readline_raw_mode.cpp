#include "readline_internal.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"

void rl_disable_raw_mode()
{
    cmp_readline_disable_raw_mode();
    return ;
}

int rl_enable_raw_mode()
{
    int enable_result;

    enable_result = cmp_readline_enable_raw_mode();
    if (enable_result == -1 && ft_errno == FT_ERR_SUCCESSS)
        ft_errno = FT_ERR_TERMINATED;
    return (enable_result);
}
