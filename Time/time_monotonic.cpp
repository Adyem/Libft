#include "time.hpp"
#include <chrono>

long    time_monotonic(void)
{
    std::chrono::steady_clock::time_point time_now = std::chrono::steady_clock::now();
    std::chrono::milliseconds milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(time_now.time_since_epoch());
    return (milliseconds.count());
}

