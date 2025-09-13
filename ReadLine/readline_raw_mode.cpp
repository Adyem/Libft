#include "readline_internal.hpp"
#include "../Compatebility/compatebility_internal.hpp"

void rl_disable_raw_mode()
{
    cmp_readline_disable_raw_mode();
    return ;
}

int rl_enable_raw_mode()
{
    return (cmp_readline_enable_raw_mode());
}
