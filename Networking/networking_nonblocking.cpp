#include "networking.hpp"
#include "../CMA/CMA.hpp"
#ifdef _WIN32
# include <winsock2.h>
#else
# include <fcntl.h>
# include <unistd.h>
# include <sys/select.h>
# ifdef __linux__
#  include <sys/epoll.h>
# endif
#endif

int nw_set_nonblocking(int socket_fd)
{
#ifdef _WIN32
    u_long mode;
    mode = 1;
    if (ioctlsocket(static_cast<SOCKET>(socket_fd), FIONBIO, &mode) != 0)
        return (-1);
    return (0);
#else
    int flags;
    flags = fcntl(socket_fd, F_GETFL, 0);
    if (flags == -1)
        return (-1);
    if (fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK) == -1)
        return (-1);
    return (0);
#endif
}

int nw_poll(int *read_file_descriptors, int read_count,
            int *write_file_descriptors, int write_count,
            int timeout_milliseconds)
{
#ifdef _WIN32
    fd_set read_set;
    fd_set write_set;
    int index;
    int max_descriptor;
    int ready_descriptors;
    int total_ready;
    timeval timeout;

    FD_ZERO(&read_set);
    FD_ZERO(&write_set);
    index = 0;
    max_descriptor = 0;
    while (read_file_descriptors && index < read_count)
    {
        FD_SET(static_cast<SOCKET>(read_file_descriptors[index]), &read_set);
        if (read_file_descriptors[index] > max_descriptor)
            max_descriptor = read_file_descriptors[index];
        index++;
    }
    index = 0;
    while (write_file_descriptors && index < write_count)
    {
        FD_SET(static_cast<SOCKET>(write_file_descriptors[index]), &write_set);
        if (write_file_descriptors[index] > max_descriptor)
            max_descriptor = write_file_descriptors[index];
        index++;
    }
    timeout.tv_sec = timeout_milliseconds / 1000;
    timeout.tv_usec = (timeout_milliseconds % 1000) * 1000;
    ready_descriptors = select(max_descriptor + 1, &read_set, &write_set, NULL, &timeout);
    if (ready_descriptors <= 0)
        return (ready_descriptors);
    index = 0;
    total_ready = 0;
    while (read_file_descriptors && index < read_count)
    {
        if (!FD_ISSET(static_cast<SOCKET>(read_file_descriptors[index]), &read_set))
            read_file_descriptors[index] = -1;
        else
            total_ready++;
        index++;
    }
    index = 0;
    while (write_file_descriptors && index < write_count)
    {
        if (!FD_ISSET(static_cast<SOCKET>(write_file_descriptors[index]), &write_set))
            write_file_descriptors[index] = -1;
        else
            total_ready++;
        index++;
    }
    return (total_ready);
#elif defined(__linux__)
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
#else
    fd_set read_set;
    fd_set write_set;
    int index;
    int max_descriptor;
    int ready_descriptors;
    int total_ready;
    timeval timeout;

    FD_ZERO(&read_set);
    FD_ZERO(&write_set);
    index = 0;
    max_descriptor = 0;
    while (read_file_descriptors && index < read_count)
    {
        FD_SET(read_file_descriptors[index], &read_set);
        if (read_file_descriptors[index] > max_descriptor)
            max_descriptor = read_file_descriptors[index];
        index++;
    }
    index = 0;
    while (write_file_descriptors && index < write_count)
    {
        FD_SET(write_file_descriptors[index], &write_set);
        if (write_file_descriptors[index] > max_descriptor)
            max_descriptor = write_file_descriptors[index];
        index++;
    }
    timeout.tv_sec = timeout_milliseconds / 1000;
    timeout.tv_usec = (timeout_milliseconds % 1000) * 1000;
    ready_descriptors = select(max_descriptor + 1, &read_set, &write_set, NULL, &timeout);
    if (ready_descriptors <= 0)
        return (ready_descriptors);
    index = 0;
    total_ready = 0;
    while (read_file_descriptors && index < read_count)
    {
        if (!FD_ISSET(read_file_descriptors[index], &read_set))
            read_file_descriptors[index] = -1;
        else
            total_ready++;
        index++;
    }
    index = 0;
    while (write_file_descriptors && index < write_count)
    {
        if (!FD_ISSET(write_file_descriptors[index], &write_set))
            write_file_descriptors[index] = -1;
        else
            total_ready++;
        index++;
    }
    return (total_ready);
#endif
}

