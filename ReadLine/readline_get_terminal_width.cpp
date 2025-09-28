#include "readline_internal.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"

int rl_get_terminal_width(void)
{
    int width;

    width = cmp_readline_terminal_width();
    if (width == -1 && ft_errno == ER_SUCCESS)
        ft_errno = FT_ETERM;
    return (width);
}
