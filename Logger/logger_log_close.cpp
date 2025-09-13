#include "logger_internal.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include <unistd.h>

void ft_log_close()
{
    size_t index;

    index = 0;
    while (index < g_sinks.size())
    {
        if (g_sinks[index].function == ft_file_sink)
        {
            s_file_sink *sink;

            sink = static_cast<s_file_sink *>(g_sinks[index].user_data);
            if (sink)
            {
                close(sink->fd);
                delete sink;
            }
        }
        else if (g_sinks[index].function == ft_syslog_sink)
        {
            cmp_syslog_close();
        }
        else if (g_sinks[index].function == ft_network_sink)
        {
            s_network_sink *sink;

            sink = static_cast<s_network_sink *>(g_sinks[index].user_data);
            if (sink)
            {
                cmp_close(sink->socket_fd);
                delete sink;
            }
        }
        index++;
    }
    g_sinks.clear();
    if (g_sinks.get_error() != ER_SUCCESS)
        return ;
    return ;
}
