#include "networking.hpp"
#include "../CMA/CMA.hpp"

void event_loop_init(event_loop *loop)
{
    loop->read_file_descriptors = NULL;
    loop->read_count = 0;
    loop->write_file_descriptors = NULL;
    loop->write_count = 0;
    return ;
}

void event_loop_clear(event_loop *loop)
{
    if (loop->read_file_descriptors)
        cma_free(loop->read_file_descriptors);
    if (loop->write_file_descriptors)
        cma_free(loop->write_file_descriptors);
    loop->read_file_descriptors = NULL;
    loop->write_file_descriptors = NULL;
    loop->read_count = 0;
    loop->write_count = 0;
    return ;
}

int event_loop_add_socket(event_loop *loop, int socket_fd, bool is_write)
{
    int *new_array;

    if (is_write)
    {
        new_array = static_cast<int *>(cma_realloc(loop->write_file_descriptors,
                                                   sizeof(int) * (loop->write_count + 1)));
        if (!new_array)
            return (-1);
        loop->write_file_descriptors = new_array;
        loop->write_file_descriptors[loop->write_count] = socket_fd;
        loop->write_count++;
    }
    else
    {
        new_array = static_cast<int *>(cma_realloc(loop->read_file_descriptors,
                                                   sizeof(int) * (loop->read_count + 1)));
        if (!new_array)
            return (-1);
        loop->read_file_descriptors = new_array;
        loop->read_file_descriptors[loop->read_count] = socket_fd;
        loop->read_count++;
    }
    return (0);
}

int event_loop_remove_socket(event_loop *loop, int socket_fd, bool is_write)
{
    int index;

    if (is_write)
    {
        index = 0;
        while (index < loop->write_count)
        {
            if (loop->write_file_descriptors[index] == socket_fd)
                break;
            index++;
        }
        if (index == loop->write_count)
            return (-1);
        while (index + 1 < loop->write_count)
        {
            loop->write_file_descriptors[index] =
                loop->write_file_descriptors[index + 1];
            index++;
        }
        loop->write_count--;
    }
    else
    {
        index = 0;
        while (index < loop->read_count)
        {
            if (loop->read_file_descriptors[index] == socket_fd)
                break;
            index++;
        }
        if (index == loop->read_count)
            return (-1);
        while (index + 1 < loop->read_count)
        {
            loop->read_file_descriptors[index] =
                loop->read_file_descriptors[index + 1];
            index++;
        }
        loop->read_count--;
    }
    return (0);
}

int event_loop_run(event_loop *loop, int timeout_milliseconds)
{
    return (nw_poll(loop->read_file_descriptors, loop->read_count,
                    loop->write_file_descriptors, loop->write_count,
                    timeout_milliseconds));
}
