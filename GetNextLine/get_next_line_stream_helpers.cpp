#include "get_next_line.hpp"

#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../CPP_class/class_string.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"
#include "../Template/move.hpp"

#include <cerrno>
#include <fcntl.h>
#include <unistd.h>

#if defined(_WIN32) || defined(_WIN64)
# include <io.h>
# define FILE_BINARY_FLAG O_BINARY
#else
# define FILE_BINARY_FLAG 0
#endif

static int append_line_to_vector(ft_vector<ft_string> &, char *line_buffer)
{
    if (line_buffer)
        cma_free(line_buffer);
    return (FT_ERR_SUCCESS);
}

int ft_read_file_lines(int fd, ft_vector<ft_string> &lines, std::size_t buffer_size)
{
    char *line_pointer;
    bool finished;
    int append_status;
    int clear_status;

    if (buffer_size == 0 || fd < 0)
        return (-1);
    finished = false;
    while (finished == false)
    {
        line_pointer = get_next_line(fd, buffer_size);
        if (!line_pointer)
        {
            finished = true;
        }
        else
        {
            append_status = append_line_to_vector(lines, line_pointer);
            if (append_status != FT_ERR_SUCCESS)
            {
                gnl_clear_stream(fd);
                return (-1);
            }
        }
    }
    clear_status = gnl_clear_stream(fd);
    if (clear_status != FT_ERR_SUCCESS)
        return (-1);
    return (0);
}

int ft_open_and_read_file(const char *path, ft_vector<ft_string> &lines, std::size_t buffer_size)
{
    int file_descriptor;
    int open_flags;
    int read_result;
    int close_result;

    if (!path)
        return (-1);
    if (buffer_size == 0)
        return (-1);
    open_flags = O_RDONLY | FILE_BINARY_FLAG;
    file_descriptor = open(path, open_flags);
    if (file_descriptor < 0)
        return (-1);
    read_result = ft_read_file_lines(file_descriptor, lines, buffer_size);
    close_result = close(file_descriptor);
    if (close_result != 0)
        return (-1);
    if (read_result != 0)
        return (-1);
    return (0);
}
