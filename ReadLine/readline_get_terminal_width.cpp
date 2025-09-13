#include "readline_internal.hpp"
#include "../Compatebility/compatebility_internal.hpp"

int rl_get_terminal_width(void)
{
    return (cmp_readline_terminal_width());
}
