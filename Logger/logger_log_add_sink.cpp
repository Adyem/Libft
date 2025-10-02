#include "logger_internal.hpp"
#include <unistd.h>
#include <cerrno>
#include "../Printf/printf.hpp"

int ft_log_add_sink(t_log_sink sink, void *user_data)
{
    if (!sink)
    {
        ft_errno = FT_EINVAL;
        return (-1);
    }
    s_log_sink entry;
    int final_error;

    g_sinks_mutex.lock(THREAD_ID);
    if (g_sinks_mutex.get_error() != ER_SUCCESS)
    {
        return (-1);
    }
    entry.function = sink;
    entry.user_data = user_data;
    g_sinks.push_back(entry);
    if (g_sinks.get_error() != ER_SUCCESS)
    {
        final_error = g_sinks.get_error();
        g_sinks_mutex.unlock(THREAD_ID);
        if (g_sinks_mutex.get_error() != ER_SUCCESS)
        {
            return (-1);
        }
        ft_errno = final_error;
        return (-1);
    }
    final_error = ER_SUCCESS;
    g_sinks_mutex.unlock(THREAD_ID);
    if (g_sinks_mutex.get_error() != ER_SUCCESS)
    {
        return (-1);
    }
    ft_errno = final_error;
    return (0);
}

void ft_json_sink(const char *message, void *user_data)
{
    char time_buffer[32];
    char level_buffer[16];
    char text_buffer[1024];
    char json_buffer[1200];
    size_t index;
    size_t dest_index;
    int fd;
    int length;

    if (!message)
        return ;
    fd = 1;
    if (user_data)
        fd = *static_cast<int *>(user_data);
    index = 1;
    dest_index = 0;
    while (message[index] && message[index] != ']')
    {
        time_buffer[dest_index] = message[index];
        index++;
        dest_index++;
    }
    time_buffer[dest_index] = '\0';
    if (!message[index])
        return ;
    index += 3;
    dest_index = 0;
    while (message[index] && message[index] != ']')
    {
        level_buffer[dest_index] = message[index];
        index++;
        dest_index++;
    }
    level_buffer[dest_index] = '\0';
    if (!message[index])
        return ;
    index += 2;
    dest_index = 0;
    while (message[index] && message[index] != '\n')
    {
        if (message[index] == '\\' || message[index] == '"')
        {
            text_buffer[dest_index] = '\\';
            dest_index++;
        }
        text_buffer[dest_index] = message[index];
        dest_index++;
        index++;
    }
    text_buffer[dest_index] = '\0';
    length = pf_snprintf(json_buffer, sizeof(json_buffer),
        "{\"time\":\"%s\",\"level\":\"%s\",\"message\":\"%s\"}\n",
        time_buffer, level_buffer, text_buffer);
    if (length > 0)
    {
        ssize_t write_result;

        write_result = write(fd, json_buffer, static_cast<size_t>(length));
        (void)write_result;
    }
    return ;
}
