#include "get_next_line.hpp"

#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
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

static int append_line_to_vector(ft_vector<ft_string> &lines, char *line_buffer)
{
    ft_string line_string(line_buffer);
    int string_error;

    string_error = ft_string::last_operation_error();
    if (line_buffer)
        cma_free(line_buffer);
    if (string_error != FT_ERR_SUCCESSS)
    {
        return (string_error);
    }
    lines.push_back(ft_move(line_string));
    if (lines.get_error() != FT_ERR_SUCCESSS)
    {
        return (lines.get_error());
    }
    return (FT_ERR_SUCCESSS);
}

int ft_read_file_lines(int fd, ft_vector<ft_string> &lines, std::size_t buffer_size)
{
    char *line_pointer;
    bool finished;
    int append_status;
    int clear_status;
    int error_code;

    if (buffer_size == 0 || fd < 0)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    finished = false;
    while (finished == false)
    {
        line_pointer = get_next_line(fd, buffer_size);
        if (!line_pointer)
        {
            error_code = ft_global_error_stack_pop_newest();
            if (error_code == FT_ERR_SUCCESSS || error_code == FT_ERR_END_OF_FILE)
            {
                finished = true;
            }
            else
            {
                gnl_clear_stream(fd);
                ft_global_error_stack_pop_newest();
                ft_global_error_stack_push(error_code);
                return (-1);
            }
        }
        else
        {
            int line_error;

            line_error = ft_global_error_stack_pop_newest();
            if (line_error != FT_ERR_SUCCESSS)
            {
                gnl_clear_stream(fd);
                ft_global_error_stack_pop_newest();
                ft_global_error_stack_push(line_error);
                if (line_pointer)
                    cma_free(line_pointer);
                return (-1);
            }
            append_status = append_line_to_vector(lines, line_pointer);
            if (append_status != FT_ERR_SUCCESSS)
            {
                gnl_clear_stream(fd);
                ft_global_error_stack_pop_newest();
                ft_global_error_stack_push(append_status);
                return (-1);
            }
        }
    }
    clear_status = gnl_clear_stream(fd);
    ft_global_error_stack_pop_newest();
    if (clear_status != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(clear_status);
        return (-1);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}

int ft_open_and_read_file(const char *path, ft_vector<ft_string> &lines, std::size_t buffer_size)
{
    int file_descriptor;
    int open_flags;
    int read_result;
    int read_error;
    int close_result;
    int close_error;
    int error_code;

    if (!path)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    if (buffer_size == 0)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    open_flags = O_RDONLY | FILE_BINARY_FLAG;
    file_descriptor = open(path, open_flags);
    if (file_descriptor < 0)
    {
        error_code = ft_map_system_error(errno);
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    read_result = ft_read_file_lines(file_descriptor, lines, buffer_size);
    read_error = ft_global_error_stack_pop_newest();
    close_result = close(file_descriptor);
    if (close_result != 0)
    {
        close_error = ft_map_system_error(errno);
        if (read_result != 0 && read_error != FT_ERR_SUCCESSS)
            error_code = read_error;
        else
            error_code = close_error;
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    if (read_result != 0)
    {
        if (read_error == FT_ERR_SUCCESSS)
            read_error = FT_ERR_IO;
        ft_global_error_stack_push(read_error);
        return (-1);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}
