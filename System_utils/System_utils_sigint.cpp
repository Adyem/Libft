#include "system_utils.hpp"
#include <csignal>

void    su_sigint(void)
{
    std::raise(SIGINT);
    return ;
}
