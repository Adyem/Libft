#include "time.hpp"
#include "../PThread/pthread.hpp"
#include <climits>
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

void    time_sleep(uint32_t seconds)
{
    uint64_t total_milliseconds;
    uint32_t chunk_milliseconds;

    total_milliseconds = static_cast<uint64_t>(seconds) * 1000ULL;
    while (total_milliseconds != 0)
    {
        if (total_milliseconds > static_cast<uint64_t>(UINT_MAX))
            chunk_milliseconds = UINT_MAX;
        else
            chunk_milliseconds = static_cast<uint32_t>(total_milliseconds);
        pt_thread_sleep(chunk_milliseconds);
        total_milliseconds -= chunk_milliseconds;
    }
    return ;
}
