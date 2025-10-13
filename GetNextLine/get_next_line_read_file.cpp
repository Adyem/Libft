#include "../Printf/printf.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../CPP_class/class_fd_istream.hpp"
#include "../System_utils/system_utils.hpp"
#include "get_next_line.hpp"
#include <unistd.h>
#include "../Errno/errno.hpp"

static char **ft_grow_lines(char **lines, int line_count, int new_capacity)
{
    int index;
    char **new_lines;

    new_lines = static_cast<char **>(cma_calloc(new_capacity + 1,
            sizeof(char *)));
    if (!new_lines)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    index = 0;
    while (index < line_count)
    {
        new_lines[index] = lines[index];
        index++;
    }
    if (lines)
        cma_free(lines);
    return (new_lines);
}

char **ft_read_file_lines(ft_istream &input, std::size_t buffer_size)
{
    char **lines = ft_nullptr;
    char *current_line = ft_nullptr;
    int line_count = 0;
    int line_capacity = 0;

    while (true)
    {
        current_line = get_next_line(input, buffer_size);
        if (!current_line)
        {
            if (ft_errno == FT_ERR_END_OF_FILE || ft_errno == ER_SUCCESS)
            {
                ft_errno = ER_SUCCESS;
                break ;
            }
            int failure_errno;

            failure_errno = ft_errno;
            gnl_clear_stream(input);
            ft_errno = failure_errno;
            cma_free_double(lines);
            return (ft_nullptr);
        }
        #ifdef DEBUG
        if (DEBUG == 1)
            pf_printf("LINE = %s", current_line);
        #endif
        if (line_count >= line_capacity)
        {
            char **new_lines;

            new_lines = ft_grow_lines(lines, line_count, line_count + 1);
            if (!new_lines)
            {
                int failure_errno;

                failure_errno = ft_errno;
                gnl_clear_stream(input);
                ft_errno = failure_errno;
                cma_free_double(lines);
                cma_free(current_line);
                return (ft_nullptr);
            }
            lines = new_lines;
            line_capacity = line_count + 1;
        }
        lines[line_count] = current_line;
        line_count++;
        lines[line_count] = ft_nullptr;
    }
    ft_errno = ER_SUCCESS;
    return (lines);
}

char **ft_open_and_read_file(const char *file_name, std::size_t buffer_size)
{
    int fd = su_open(file_name);
    ft_fd_istream input(fd);
    char **lines;

    if (fd < 0)
    {
        if (ft_errno == ER_SUCCESS)
            ft_errno = FT_ERR_INVALID_HANDLE;
        return (ft_nullptr);
    }
    lines = ft_read_file_lines(input, buffer_size);
    close(fd);
    return (lines);
}
