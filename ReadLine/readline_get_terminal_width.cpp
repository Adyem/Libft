#include "readline_internal.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"

int rl_get_terminal_width(void)
{
    static terminal_dimensions shared_dimensions;
    unsigned short             rows;
    unsigned short             cols;
    bool                       dimensions_valid;

    if (rl_terminal_dimensions_prepare_thread_safety(&shared_dimensions) != 0)
        return (-1);
    if (rl_terminal_dimensions_refresh(&shared_dimensions) != 0)
        return (-1);
    if (rl_terminal_dimensions_get(&shared_dimensions, &rows, &cols, ft_nullptr,
        ft_nullptr, &dimensions_valid) != 0)
        return (-1);
    if (dimensions_valid == false)
    {
        ft_errno = FT_ERR_TERMINATED;
        return (-1);
    }
    ft_errno = FT_ER_SUCCESSS;
    return (static_cast<int>(cols));
}
