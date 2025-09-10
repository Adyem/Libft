#include "../Printf/printf.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../CPP_class/class_fd_istream.hpp"
#include "../System_utils/system_utils.hpp"
#include "get_next_line.hpp"
#include <unistd.h>

static void ft_handle_allocation_failure(char **lines)
{
    int index = 0;

    if (lines)
    {
        while (lines[index])
        {
            cma_free(lines[index]);
            index++;
        }
        cma_free(lines);
    }
    return ;
}

static char **ft_reallocate_lines(char **lines, int new_size)
{
    int index = 0;
    char **new_lines = static_cast<char **>(cma_calloc(new_size + 1, sizeof(char *)));

    if (!new_lines)
    {
        ft_handle_allocation_failure(lines);
        return (ft_nullptr);
    }

    if (lines)
    {
        while (lines[index])
        {
            new_lines[index] = lines[index];
            index++;
        }
    }
    cma_free(lines);
    return (new_lines);
}

char **ft_read_file_lines(ft_istream &input, std::size_t buffer_size)
{
    char **lines = ft_nullptr;
    char *current_line = ft_nullptr;
    int line_count = 0;

    while (true)
    {
        current_line = get_next_line(input, buffer_size);
        if (!current_line)
            break ;
        #ifdef DEBUG
        if (DEBUG == 1)
            pf_printf("LINE = %s", current_line);
        #endif
        line_count++;
        lines = ft_reallocate_lines(lines, line_count);
        if (!lines)
        {
            cma_free(current_line);
            return (ft_nullptr);
        }
        lines[line_count - 1] = current_line;
    }
    return (lines);
}

char **ft_open_and_read_file(const char *file_name, std::size_t buffer_size)
{
    int fd = su_open(file_name);
    ft_fd_istream input(fd);
    char **lines;

    if (fd < 0)
        return (ft_nullptr);
    lines = ft_read_file_lines(input, buffer_size);
    close(fd);
    return (lines);
}
