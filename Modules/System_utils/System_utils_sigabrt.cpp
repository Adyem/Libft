#include "system_utils.hpp"
#include <cstdlib>

void    su_sigabrt(void)
{
    std::exit(EXIT_FAILURE);
    return ;
}
