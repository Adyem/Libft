#include "logger_internal.hpp"
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
        index++;
    }
    g_sinks.clear();
    return ;
}
