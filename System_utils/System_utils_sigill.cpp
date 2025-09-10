#include "system_utils.hpp"
#include <csignal>

void    su_sigill(void)
{
    std::raise(SIGILL);
    return ;
}
