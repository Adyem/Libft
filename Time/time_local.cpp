#include "time.hpp"
#include <ctime>

void    time_local(time_t time_value, struct tm *out)
{
    if (!out)
        return ;
    std::tm *tmp = std::localtime(&time_value);
    if (!tmp)
        return ;
    *out = *tmp;
    return ;
}

