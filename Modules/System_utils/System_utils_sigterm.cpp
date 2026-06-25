#include "system_utils.hpp"
#include <csignal>
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

void    su_sigterm(void)
{
    std::raise(SIGTERM);
    return ;
}
