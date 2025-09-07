#include "time.hpp"
#include <thread>
#include <chrono>

void    time_sleep_ms(unsigned int milliseconds)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    return ;
}

