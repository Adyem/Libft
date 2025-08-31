#include "logger_internal.hpp"
#include <unistd.h>

void ft_log_close()
{
    if (!g_path.empty())
        close(g_fd);
    g_fd = 1;
    g_path.clear();
    g_max_size = 0;
}
