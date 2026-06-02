#include "time.hpp"
#include "../PThread/pthread.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

void    time_sleep_ms(uint32_t milliseconds)
{
    pt_thread_sleep(milliseconds);
    return ;
}

