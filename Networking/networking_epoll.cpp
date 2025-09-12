#include "networking.hpp"
#include "../CMA/CMA.hpp"
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
    int ready_descriptors;
    int ready_index;
    int descriptor;
    int search_index;

    epoll_descriptor = epoll_create1(0);
    if (epoll_descriptor == -1)
        return (-1);
    index = 0;
    while (read_file_descriptors && index < read_count)
    {
        event.events = EPOLLIN;
        event.data.fd = read_file_descriptors[index];
        if (epoll_ctl(epoll_descriptor, EPOLL_CTL_ADD, read_file_descriptors[index], &event) == -1)
        {
            close(epoll_descriptor);
            return (-1);
        }
        index++;
    }
    index = 0;
    while (write_file_descriptors && index < write_count)
    {
        event.events = EPOLLOUT;
        event.data.fd = write_file_descriptors[index];
        if (epoll_ctl(epoll_descriptor, EPOLL_CTL_ADD, write_file_descriptors[index], &event) == -1)
        {
            close(epoll_descriptor);
            return (-1);
        }
        index++;
    }
    maximum_events = read_count + write_count;
    if (maximum_events == 0)
    {
        close(epoll_descriptor);
        return (0);
    }
    events = static_cast<epoll_event *>(cma_malloc(sizeof(epoll_event) * maximum_events));
    if (!events)
    {
        close(epoll_descriptor);
        return (-1);
    }
    ready_descriptors = epoll_wait(epoll_descriptor, events, maximum_events, timeout_milliseconds);
    if (ready_descriptors <= 0)
    {
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
            read_file_descriptors[search_index] = descriptor;
        search_index = 0;
        while (write_file_descriptors && search_index < write_count)
        {
            if (write_file_descriptors[search_index] == descriptor)
                break;
            search_index++;
        }
        if (write_file_descriptors && search_index < write_count)
            write_file_descriptors[search_index] = descriptor;
        ready_index++;
    }
    cma_free(events);
    close(epoll_descriptor);
    return (ready_descriptors);
}
