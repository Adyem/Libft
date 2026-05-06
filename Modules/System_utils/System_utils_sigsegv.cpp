#include "system_utils.hpp"
#include <csignal>

void    su_sigsegv(void)
{
    std::raise(SIGSEGV);
    return ;
}
