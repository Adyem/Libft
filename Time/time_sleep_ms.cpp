#include "time.hpp"
#include "../PThread/pthread.hpp"

void    time_sleep_ms(unsigned int milliseconds)
{
    pt_thread_sleep(milliseconds);
    return ;
}

