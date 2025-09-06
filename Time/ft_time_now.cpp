#include "time.hpp"
#include <ctime>

time_t  ft_time_now(void)
{
    return (std::time(nullptr));
}

