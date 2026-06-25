#include "readline_internal.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

int32_t rl_disable_raw_mode()
{
    return (cmp_readline_disable_raw_mode());
}

int32_t rl_enable_raw_mode()
{
    return (cmp_readline_enable_raw_mode());
}
