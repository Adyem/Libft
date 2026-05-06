#include "system_utils.hpp"
#include <csignal>

void    su_sigterm(void)
{
    std::raise(SIGTERM);
    return ;
}
