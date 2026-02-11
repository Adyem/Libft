#include "readline_internal.hpp"
#include "../Compatebility/compatebility_internal.hpp"

int rl_disable_raw_mode()
{
    return (cmp_readline_disable_raw_mode());
}

int rl_enable_raw_mode()
{
    return (cmp_readline_enable_raw_mode());
}
