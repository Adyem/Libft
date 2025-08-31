#include "logger_internal.hpp"
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>

void ft_log_rotate()
{
    if (g_path.empty() || g_max_size == 0)
        return;
    struct stat st;
    if (fstat(g_fd, &st) == -1)
        return;
    if (static_cast<size_t>(st.st_size) < g_max_size)
        return;
    close(g_fd);
    std::string rotated = g_path + ".1";
    std::rename(g_path.c_str(), rotated.c_str());
    g_fd = open(g_path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
}
