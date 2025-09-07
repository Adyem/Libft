#include "time.hpp"
#include <chrono>

long    time_now_ms(void)
{
    std::chrono::system_clock::time_point time_now = std::chrono::system_clock::now();
    std::chrono::milliseconds milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(time_now.time_since_epoch());
    return (milliseconds.count());
}

