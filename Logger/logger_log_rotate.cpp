#include "logger_internal.hpp"
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>

void ft_log_rotate(s_file_sink *sink)
{
    struct stat   st;
    std::string   rotated;

    if (!sink)
        return ;
    if (sink->path.empty() || sink->max_size == 0)
        return ;
    if (fstat(sink->fd, &st) == -1)
        return ;
    if (static_cast<size_t>(st.st_size) < sink->max_size)
        return ;
    close(sink->fd);
    rotated = sink->path + ".1";
    std::rename(sink->path.c_str(), rotated.c_str());
    sink->fd = open(sink->path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
    return ;
}
