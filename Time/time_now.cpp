#include "time.hpp"
#include <ctime>

time_t  time_now(void)
{
    return (std::time(nullptr));
}

