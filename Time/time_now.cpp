#include "time.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include <ctime>
#include <cerrno>

t_time  time_now(void)
{
    std::time_t standard_time;
    int error_code;

    standard_time = ::time(ft_nullptr);
    if (standard_time == static_cast<std::time_t>(-1))
    {
        error_code = errno;
        if (error_code != 0)
            error_code = ft_map_system_error(error_code);
        else
            error_code = FT_ERR_TERMINATED;
        ft_global_error_stack_push(error_code);
        return (static_cast<t_time>(-1));
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (static_cast<t_time>(standard_time));
}
