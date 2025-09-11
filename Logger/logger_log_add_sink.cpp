#include "logger_internal.hpp"

int ft_log_add_sink(t_log_sink sink, void *user_data)
{
    if (!sink)
        return (-1);
    s_log_sink entry;
    entry.function = sink;
    entry.user_data = user_data;
    g_sinks.push_back(entry);
    if (g_sinks.get_error() != ER_SUCCESS)
        return (-1);
    return (0);
}
