#include "time.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include <ctime>
#include <cerrno>

t_time  time_now(void)
{
    std::time_t standard_time;
    int saved_errno;

    standard_time = ::time(ft_nullptr);
    if (standard_time == static_cast<std::time_t>(-1))
    {
        saved_errno = errno;
        if (saved_errno != 0)
            ft_errno = ft_map_system_error(saved_errno);
        else
            ft_errno = FT_ERR_TERMINATED;
        return (static_cast<t_time>(-1));
    }
    ft_errno = FT_ER_SUCCESSS;
    return (static_cast<t_time>(standard_time));
}

