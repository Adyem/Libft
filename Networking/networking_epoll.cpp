#include "networking.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include <unistd.h>
#include <sys/epoll.h>

int nw_poll(int *read_file_descriptors, int read_count,
            int *write_file_descriptors, int write_count,
            int timeout_milliseconds)
{
    int epoll_descriptor;
    epoll_event event;
    epoll_event *events;
    int index;
    int maximum_events;
    int valid_read_count;
    int valid_write_count;
    int ready_descriptors;
    int ready_index;
    int descriptor;
    int search_index;
    int *read_ready_flags;
    int *write_ready_flags;

    epoll_descriptor = epoll_create1(0);
    if (epoll_descriptor == -1)
    {
        return (-1);
    }
    index = 0;
    valid_read_count = 0;
    while (read_file_descriptors && index < read_count)
    {
        if (read_file_descriptors[index] >= 0)
        {
            event.events = EPOLLIN;
            event.data.fd = read_file_descriptors[index];
            if (epoll_ctl(epoll_descriptor, EPOLL_CTL_ADD, read_file_descriptors[index], &event) == -1)
            {
                close(epoll_descriptor);
                return (-1);
            }
            valid_read_count++;
        }
        index++;
    }
    index = 0;
    valid_write_count = 0;
    while (write_file_descriptors && index < write_count)
    {
        if (write_file_descriptors[index] >= 0)
        {
            event.events = EPOLLOUT;
            event.data.fd = write_file_descriptors[index];
            if (epoll_ctl(epoll_descriptor, EPOLL_CTL_ADD, write_file_descriptors[index], &event) == -1)
            {
                close(epoll_descriptor);
                return (-1);
            }
            valid_write_count++;
        }
        index++;
    }
    maximum_events = valid_read_count + valid_write_count;
    if (maximum_events == 0)
    {
        index = 0;
        while (read_file_descriptors && index < read_count)
        {
            read_file_descriptors[index] = -1;
            index++;
        }
        index = 0;
        while (write_file_descriptors && index < write_count)
        {
            write_file_descriptors[index] = -1;
            index++;
        }
        close(epoll_descriptor);
        return (0);
    }
    events = static_cast<epoll_event *>(cma_malloc(sizeof(epoll_event) * maximum_events));
    if (!events)
    {
        close(epoll_descriptor);
        return (-1);
    }
    read_ready_flags = ft_nullptr;
    write_ready_flags = ft_nullptr;
    if (read_file_descriptors && read_count > 0)
    {
        read_ready_flags = static_cast<int *>(cma_malloc(sizeof(int) * read_count));
        if (!read_ready_flags)
        {
            cma_free(events);
            close(epoll_descriptor);
            return (-1);
        }
        index = 0;
        while (index < read_count)
        {
            read_ready_flags[index] = 0;
            index++;
        }
    }
    if (write_file_descriptors && write_count > 0)
    {
        write_ready_flags = static_cast<int *>(cma_malloc(sizeof(int) * write_count));
        if (!write_ready_flags)
        {
            if (read_ready_flags)
                cma_free(read_ready_flags);
            cma_free(events);
            close(epoll_descriptor);
            return (-1);
        }
        index = 0;
        while (index < write_count)
        {
            write_ready_flags[index] = 0;
            index++;
        }
    }
    ready_descriptors = epoll_wait(epoll_descriptor, events, maximum_events, timeout_milliseconds);
    if (ready_descriptors <= 0)
    {
        if (read_ready_flags)
            cma_free(read_ready_flags);
        if (write_ready_flags)
            cma_free(write_ready_flags);
        cma_free(events);
        close(epoll_descriptor);
        return (ready_descriptors);
    }
    ready_index = 0;
    while (ready_index < ready_descriptors)
    {
        descriptor = events[ready_index].data.fd;
        search_index = 0;
        while (read_file_descriptors && search_index < read_count)
        {
            if (read_file_descriptors[search_index] == descriptor)
                break;
            search_index++;
        }
        if (read_file_descriptors && search_index < read_count)
        {
            read_file_descriptors[search_index] = descriptor;
            if (read_ready_flags)
                read_ready_flags[search_index] = 1;
        }
        search_index = 0;
        while (write_file_descriptors && search_index < write_count)
        {
            if (write_file_descriptors[search_index] == descriptor)
                break;
            search_index++;
        }
        if (write_file_descriptors && search_index < write_count)
        {
            write_file_descriptors[search_index] = descriptor;
            if (write_ready_flags)
                write_ready_flags[search_index] = 1;
        }
        ready_index++;
    }
    index = 0;
    while (read_file_descriptors && index < read_count)
    {
        if (!read_ready_flags || read_ready_flags[index] == 0)
            read_file_descriptors[index] = -1;
        index++;
    }
    index = 0;
    while (write_file_descriptors && index < write_count)
    {
        if (!write_ready_flags || write_ready_flags[index] == 0)
            write_file_descriptors[index] = -1;
        index++;
    }
    cma_free(events);
    if (read_ready_flags)
        cma_free(read_ready_flags);
    if (write_ready_flags)
        cma_free(write_ready_flags);
    close(epoll_descriptor);
    return (ready_descriptors);
}
