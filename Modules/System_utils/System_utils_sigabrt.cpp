#include "system_utils.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

void    su_sigabrt(void)
{
    su_abort();
    return ;
}
