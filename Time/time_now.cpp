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
            ft_errno = saved_errno + ERRNO_OFFSET;
        else
            ft_errno = FT_ETERM;
        return (static_cast<t_time>(-1));
    }
    ft_errno = ER_SUCCESS;
    return (static_cast<t_time>(standard_time));
}

