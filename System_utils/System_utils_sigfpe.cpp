#include "system_utils.hpp"
#include <csignal>

void    su_sigfpe(void)
{
    std::raise(SIGFPE);
    return ;
}
