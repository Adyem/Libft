#include "time.hpp"
#include "../PThread/pthread.hpp"
#include <climits>

void    time_sleep(unsigned int seconds)
{
    unsigned long long total_milliseconds;
    unsigned int chunk_milliseconds;

    total_milliseconds = static_cast<unsigned long long>(seconds) * 1000ULL;
    while (total_milliseconds != 0)
    {
        if (total_milliseconds > static_cast<unsigned long long>(UINT_MAX))
            chunk_milliseconds = UINT_MAX;
        else
            chunk_milliseconds = static_cast<unsigned int>(total_milliseconds);
        pt_thread_sleep(chunk_milliseconds);
        total_milliseconds -= chunk_milliseconds;
    }
    return ;
}

