#include "logger_internal.hpp"

void ft_log_remove_sink(t_log_sink sink, void *user_data)
{
    size_t index;
    bool   removed;

    index = 0;
    removed = false;
    while (index < g_sinks.size())
    {
        if (g_sinks[index].function == sink && g_sinks[index].user_data == user_data)
        {
            g_sinks.erase(g_sinks.begin() + index);
            if (g_sinks.get_error() != ER_SUCCESS)
            {
                ft_errno = g_sinks.get_error();
                return ;
            }
            removed = true;
            break;
        }
        index++;
    }
    if (!removed)
        ft_errno = FT_EINVAL;
    else
        ft_errno = ER_SUCCESS;
    return ;
}
