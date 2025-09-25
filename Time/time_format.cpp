#include "time.hpp"
#include "../CPP_class/class_string_class.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/mutex.hpp"
#include <ctime>

ft_string    time_format_iso8601(t_time time_value)
{
    std::time_t standard_time;
    std::tm time_storage;
    std::tm *time_pointer;
    char buffer[21];
    ft_string formatted;
    static pt_mutex g_gmtime_mutex;

    standard_time = static_cast<std::time_t>(time_value);
    if (g_gmtime_mutex.lock(THREAD_ID) != FT_SUCCESS)
        return (ft_string());
    time_pointer = std::gmtime(&standard_time);
    if (!time_pointer)
    {
        if (g_gmtime_mutex.unlock(THREAD_ID) != FT_SUCCESS)
            return (ft_string());
        return (ft_string());
    }
    time_storage = *time_pointer;
    if (g_gmtime_mutex.unlock(THREAD_ID) != FT_SUCCESS)
        return (ft_string());
    if (std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &time_storage) == 0)
        return (ft_string());
    formatted = ft_string(buffer);
    return (formatted);
}

