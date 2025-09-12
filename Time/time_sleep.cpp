#include "time.hpp"
#include "../PThread/pthread.hpp"

void    time_sleep(unsigned int seconds)
{
    pt_thread_sleep(seconds * 1000);
    return ;
}

