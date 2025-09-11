#include "time.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include <ctime>

t_time  time_now(void)
{
    std::time_t standard_time;

    standard_time = std::time(ft_nullptr);
    return (static_cast<t_time>(standard_time));
}

