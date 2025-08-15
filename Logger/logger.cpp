#include "logger.hpp"
#include <unistd.h>
#include <cstring>

static void ft_log_write(int fd, const char *prefix, const char *message)
{
    if (!message)
        return ;
    write(fd, prefix, std::strlen(prefix));
    write(fd, message, std::strlen(message));
    write(fd, "\n", 1);
}

void ft_log_info(int fd, const char *message)
{
    ft_log_write(fd, "[INFO] ", message);
}

void ft_log_warn(int fd, const char *message)
{
    ft_log_write(fd, "[WARN] ", message);
}

void ft_log_error(int fd, const char *message)
{
    ft_log_write(fd, "[ERROR] ", message);
}

