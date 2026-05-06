#include "time.hpp"
#include "../PThread/pthread.hpp"

void    time_sleep_ms(uint32_t milliseconds)
{
    pt_thread_sleep(milliseconds);
    return ;
}

