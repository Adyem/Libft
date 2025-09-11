#include "time.hpp"
#include "../CPP_class/class_string_class.hpp"
#include <ctime>

ft_string    time_format_iso8601(t_time time_value)
{
    std::time_t standard_time;
    std::tm *time_pointer;
    char buffer[21];
    ft_string formatted;

    standard_time = static_cast<std::time_t>(time_value);
    time_pointer = std::gmtime(&standard_time);
    if (!time_pointer)
        return (ft_string());
    if (std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", time_pointer) == 0)
        return (ft_string());
    formatted = ft_string(buffer);
    return (formatted);
}

