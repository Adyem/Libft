#include "system_utils.hpp"
#include <csignal>

void    su_sigabrt(void)
{
    std::raise(SIGABRT);
    return ;
}
