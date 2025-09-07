#include "time.hpp"
#include <thread>
#include <chrono>

void    time_sleep(unsigned int seconds)
{
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
    return ;
}

