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
        if (error_code == 0)
            error_code = FT_ERR_TERMINATED;
        (void)(error_code);
        return (static_cast<t_time>(-1));
    }
    return (static_cast<t_time>(standard_time));
}
